/*
* Este código es de dominio público
* Realizado por Hugo Montañés García
*/

#include "Scene.hpp"

#include <iostream>
#include <cassert>
#include "HeightMapTerrain.hpp"


namespace space
{

	Scene::Scene(unsigned width, unsigned height)
		: angle(0.0f)
	{
		glEnable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);

		shader_program = std::make_unique<ShaderProgram>();

		VertexShader vertex_shader;
		if (!vertex_shader.loadFromFile("../../../shared/assets/shaders/vertex/vertex_shader.glsl"))
		{
			throw std::runtime_error("Failed to load vertex shader.");
		}

		FragmentShader fragment_shader;
		if (!fragment_shader.loadFromFile("../../../shared/assets/shaders/fragment/fragment_shader.glsl"))
		{
			throw std::runtime_error("Failed to load fragment shader.");
		}

		shader_program->attachShader(vertex_shader);
		shader_program->attachShader(fragment_shader);

		if (!shader_program->link())
		{
			throw std::runtime_error("Failed to link shader program.");
		}

		shader_program->detachAndDeleteShaders({ vertex_shader, fragment_shader });

		shader_program->use();

		// Get uniform locations
		model_view_matrix_id = glGetUniformLocation(shader_program->getProgramID(), "model_view_matrix");
		normal_matrix_id = glGetUniformLocation(shader_program->getProgramID(), "normal_matrix");
		projection_matrix_id = glGetUniformLocation(shader_program->getProgramID(), "projection_matrix");

		//Root node
		root = std::make_shared<SceneNode>("root");

		//Setup camera
		activeCamera = std::make_shared<Camera>("main_camera");
		activeCamera->position = glm::vec3(0, 20, 50);
		activeCamera->rotation = glm::vec3(-0.4f, 0.0f, 0.0f);
		root->addChild(activeCamera);

		auto terrainNode = std::make_shared<SceneNode>("main_terrain");
		auto terrainMesh = std::make_shared<HeightMapTerrain>(
			"../../../shared/assets/textures/heightmaps/heightmap_010.png",
			1.0f  // height scale
		);
		terrainNode->mesh = terrainMesh;
		terrainNode->position = glm::vec3(0, 15, 20);
		terrainNode->scale = glm::vec3(1.0f);
		root->addChild(terrainNode);

		/*auto planeNode = std::make_shared<SceneNode>("plane");
		planeNode->mesh = std::make_shared<Plane>(5, 5, 10.0f, 10.0f);
		planeNode->position = glm::vec3(0, -2, 0);
		root->addChild(planeNode);

		auto coneNode = std::make_shared<SceneNode>("cone");
		coneNode->mesh = std::make_shared<Cone>(100);
		coneNode->position = glm::vec3(0, 1, 0);
		planeNode->addChild(coneNode);*/

		//Initialize transparent shader
		transparent_shader = std::make_unique<ShaderProgram>();

		VertexShader trans_vertex_shader;
		if (!trans_vertex_shader.loadFromFile("../../../shared/assets/shaders/vertex/vertex_shader.glsl"))
		{
			throw std::runtime_error("Failed to load transparent vertex shader.");
		}

		FragmentShader trans_fragment_shader;
		if (!trans_fragment_shader.loadFromFile("../../../shared/assets/shaders/fragment/fragment_shader_transparent.glsl"))
		{
			throw std::runtime_error("Failed to load transparent fragment shader.");
		}

		transparent_shader->attachShader(trans_vertex_shader);
		transparent_shader->attachShader(trans_fragment_shader);

		if (!transparent_shader->link())
		{
			throw std::runtime_error("Failed to link transparent shader program.");
		}

		transparent_shader->detachAndDeleteShaders({ trans_vertex_shader, trans_fragment_shader });

		// Get uniform locations for transparent shader
		transparent_shader->use();
		transparent_model_view_matrix_id = glGetUniformLocation(transparent_shader->getProgramID(), "model_view_matrix");
		transparent_normal_matrix_id = glGetUniformLocation(transparent_shader->getProgramID(), "normal_matrix");
		transparent_projection_matrix_id = glGetUniformLocation(transparent_shader->getProgramID(), "projection_matrix");
		transparency_uniform_id = glGetUniformLocation(transparent_shader->getProgramID(), "transparency");

		// Set default transparency
		glUniform1f(transparency_uniform_id, 0.3f); // 30% opacity

		// Create the transparent cube around the terrain
		transparentCubeNode = std::make_shared<SceneNode>("transparent_cube");

		// Calculate cube dimensions to encompass the terrain
		// Position it at the same location as terrain but make it larger
		float cubeSize = 25.0f; // Slightly larger than terrain's 20 unit span
		float cubeHeight = 15.0f; // To cover the terrain's height variations

		transparentCubeNode->mesh = std::make_shared<Cube>(cubeSize, cubeHeight, cubeSize);
		transparentCubeNode->position = glm::vec3(0, 20, 20); // Center around terrain
		transparentCubeNode->scale = glm::vec3(1.0f);

		root->addChild(transparentCubeNode);

		// Initialize grass shader
		grass_shader = std::make_unique<ShaderProgram>();

		VertexShader grass_vertex_shader;
		if (!grass_vertex_shader.loadFromFile("../../../shared/assets/shaders/vertex/grass_vertex_shader.glsl"))
		{
			throw std::runtime_error("Failed to load grass vertex shader.");
		}

		// Can reuse the same fragment shader as terrain
		FragmentShader grass_fragment_shader;
		if (!grass_fragment_shader.loadFromFile("../../../shared/assets/shaders/fragment/fragment_shader.glsl"))
		{
			throw std::runtime_error("Failed to load grass fragment shader.");
		}

		grass_shader->attachShader(grass_vertex_shader);
		grass_shader->attachShader(grass_fragment_shader);

		if (!grass_shader->link())
		{
			throw std::runtime_error("Failed to link grass shader program.");
		}

		grass_shader->detachAndDeleteShaders({ grass_vertex_shader, grass_fragment_shader });

		// Get uniform locations for grass shader
		grass_shader->use();
		grass_model_view_matrix_id = glGetUniformLocation(grass_shader->getProgramID(), "model_view_matrix");
		grass_normal_matrix_id = glGetUniformLocation(grass_shader->getProgramID(), "normal_matrix");
		grass_projection_matrix_id = glGetUniformLocation(grass_shader->getProgramID(), "projection_matrix");

		// Create grass on the terrain
		if (terrainMesh && terrainNode)
		{
			// Get the terrain's world transform matrix
			glm::mat4 terrainTransform = terrainNode->getWorldTransform();

			// Create grass using the terrain-aware method
			grassMesh = terrainMesh->createGrassForTerrain(
				terrainTransform,
				"../../../shared/assets/models/SM_Grass.fbx",
				500000
			);

			if (grassMesh)
			{
				std::cout << "Successfully created " << grassMesh->getInstanceCount()
					<< " grass instances on terrain" << std::endl;

				// Debug: Print some grass positions to verify placement
				if (grassMesh->getInstanceCount() > 0)
				{
					std::cout << "Terrain transform: " << std::endl;
					std::cout << "  Position: " << terrainNode->position.x << ", "
						<< terrainNode->position.y << ", " << terrainNode->position.z << std::endl;
					std::cout << "  Scale: " << terrainNode->scale.x << ", "
						<< terrainNode->scale.y << ", " << terrainNode->scale.z << std::endl;
				}
			}
			else
			{
				std::cerr << "Failed to create grass instances" << std::endl;
			}
		}

		if (grassMesh) {
			grassMesh->printStatistics();

			// Also print terrain info
			std::cout << "Terrain info:" << std::endl;
			std::cout << "  Local scale: " << terrainMesh->getTerrainWorldScale() << std::endl;
			std::cout << "  Scene scale: " << terrainNode->scale.x << std::endl;
			std::cout << "  World span: " << (terrainMesh->getTerrainWorldScale() * terrainNode->scale.x) << std::endl;
		}

		// Initialize skybox shader
		skybox_shader = std::make_unique<ShaderProgram>();

		VertexShader skybox_vertex_shader;
		if (!skybox_vertex_shader.loadFromFile("../../../shared/assets/shaders/vertex/skybox_vertex_shader.glsl"))
		{
			throw std::runtime_error("Failed to load skybox vertex shader.");
		}

		FragmentShader skybox_fragment_shader;
		if (!skybox_fragment_shader.loadFromFile("../../../shared/assets/shaders/fragment/skybox_fragment_shader.glsl"))
		{
			throw std::runtime_error("Failed to load skybox fragment shader.");
		}

		skybox_shader->attachShader(skybox_vertex_shader);
		skybox_shader->attachShader(skybox_fragment_shader);

		if (!skybox_shader->link())
		{
			throw std::runtime_error("Failed to link skybox shader program.");
		}

		skybox_shader->detachAndDeleteShaders({ skybox_vertex_shader, skybox_fragment_shader });

		skybox_shader->use();
		skybox_view_matrix_id = glGetUniformLocation(skybox_shader->getProgramID(), "view_matrix");
		skybox_projection_matrix_id = glGetUniformLocation(skybox_shader->getProgramID(), "projection_matrix");

		// Set the skybox sampler to use texture unit 0
		glUniform1i(glGetUniformLocation(skybox_shader->getProgramID(), "skybox"), 0);

		// Create and load the skybox
		std::vector<std::string> faces = {
			"../../../shared/assets/textures/skybox/right.jpg",
			"../../../shared/assets/textures/skybox/left.jpg",
			"../../../shared/assets/textures/skybox/top.jpg",
			"../../../shared/assets/textures/skybox/bottom.jpg",
			"../../../shared/assets/textures/skybox/front.jpg",
			"../../../shared/assets/textures/skybox/back.jpg"
		};

		skybox = std::make_shared<Skybox>(faces);

		// Enable depth testing
		glEnable(GL_DEPTH_TEST);

		resize(width, height);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "OpenGL error in Scene constructor: " << error << std::endl;
		}
	}

	void Scene::update(float deltaTime)
	{
		//angle += 0.01f;

		 // Update the cube's rotation angle
		cubeRotationAngle += cubeRotationSpeed * deltaTime;

		// Keep the angle in bounds (prevents floating point overflow after long run times)
		while (cubeRotationAngle > 2.0f * glm::pi<float>()) {
			cubeRotationAngle -= 2.0f * glm::pi<float>();
		}
		while (cubeRotationAngle < 0.0f) {
			cubeRotationAngle += 2.0f * glm::pi<float>();
		}

		// Apply rotation to the cube
		if (transparentCubeNode) {
			transparentCubeNode->rotation = glm::vec3(0.0f, cubeRotationAngle, 0.0f);
		}

		//Get current keyboard state
		const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
		handleKeyboard(keyboardState);

		updateCamera(deltaTime);

		//handleRotationControls(keyboardState);
		updateTransparencyAnimation(deltaTime);

	}

	void Scene::renderNode(const std::shared_ptr<SceneNode>& node, const glm::mat4& viewMatrix)
	{
		if (node->mesh)
		{
			shader_program->use();

			// Calculate matrices
			glm::mat4 model_matrix = node->getWorldTransform();
			glm::mat4 model_view_matrix = viewMatrix * model_matrix;
			glm::mat4 normal_matrix = glm::transpose(glm::inverse(model_view_matrix));

			// Send matrices to shader
			glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));
			glUniformMatrix4fv(normal_matrix_id, 1, GL_FALSE, glm::value_ptr(normal_matrix));

			// Render the mesh
			node->mesh->render();
		}

		// Recursively render children
		for (const auto& child : node->children) {
			renderNode(child, viewMatrix);
		}
		
	}

	void Scene::render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!activeCamera) return;

		// Get camera matrices
		glm::mat4 view_matrix = activeCamera->getViewMatrix();
		glm::mat4 projection_matrix = activeCamera->getProjectionMatrix();

		// ===== STEP 1: Render Skybox (special case - rendered first) =====
		// The skybox is rendered with depth testing in a special way
		skybox_shader->use();

		// Remove translation from the view matrix to keep skybox centered around camera
		glm::mat4 skybox_view = glm::mat4(glm::mat3(view_matrix));

		glUniformMatrix4fv(skybox_view_matrix_id, 1, GL_FALSE, glm::value_ptr(skybox_view));
		glUniformMatrix4fv(skybox_projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		skybox->render();

		// ===== STEP 2: Render All Opaque Objects =====
		// This includes terrain and any non-transparent objects
		shader_program->use();
		glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Render all opaque objects in the scene graph
		// We'll modify renderNode to skip transparent objects
		renderOpaqueNodes(root, view_matrix);

		// Render grass (also opaque)
		if (grassMesh && grassMesh->getInstanceCount() > 0)
		{
			grass_shader->use();

			glm::mat4 model_matrix = glm::mat4(1.0f);
			glm::mat4 model_view_matrix = view_matrix * model_matrix;
			glm::mat4 normal_matrix = glm::transpose(glm::inverse(model_view_matrix));

			glUniformMatrix4fv(grass_model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));
			glUniformMatrix4fv(grass_normal_matrix_id, 1, GL_FALSE, glm::value_ptr(normal_matrix));
			glUniformMatrix4fv(grass_projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

			grassMesh->render();
		}

		// ===== STEP 3: Set up for Transparency Rendering =====
		// Enable blending for transparency
		glEnable(GL_BLEND);
		// This is the standard transparency blending function
		// It means: final_color = source_color * source_alpha + dest_color * (1 - source_alpha)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_FALSE);

		// ===== STEP 4: Render Transparent Objects =====
		// Now render the transparent cube
		if (transparentCubeNode && transparentCubeNode->mesh)
		{
			transparent_shader->use();
			glUniformMatrix4fv(transparent_projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

			// Calculate matrices for the transparent cube
			glm::mat4 model_matrix = transparentCubeNode->getWorldTransform();
			glm::mat4 model_view_matrix = view_matrix * model_matrix;
			glm::mat4 normal_matrix = glm::transpose(glm::inverse(model_view_matrix));

			// Send matrices to shader
			glUniformMatrix4fv(transparent_model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));
			glUniformMatrix4fv(transparent_normal_matrix_id, 1, GL_FALSE, glm::value_ptr(normal_matrix));

			// Render the transparent cube
			transparentCubeNode->mesh->render();
		}

		// ===== STEP 5: Restore OpenGL State =====
		// Re-enable depth writing for next frame
		glDepthMask(GL_TRUE);
		// Disable blending
		glDisable(GL_BLEND);

		// Check for any OpenGL errors
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "OpenGL error in render: " << error << std::endl;
		}
	}

	// Helper method to render only opaque nodes
	void Scene::renderOpaqueNodes(const std::shared_ptr<SceneNode>& node, const glm::mat4& viewMatrix)
	{
		// Skip transparent objects during opaque pass
		if (node->name == "transparent_cube") {
			// Skip this node and its children
			return;
		}

		if (node->mesh)
		{
			shader_program->use();

			// Calculate matrices
			glm::mat4 model_matrix = node->getWorldTransform();
			glm::mat4 model_view_matrix = viewMatrix * model_matrix;
			glm::mat4 normal_matrix = glm::transpose(glm::inverse(model_view_matrix));

			// Send matrices to shader
			glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));
			glUniformMatrix4fv(normal_matrix_id, 1, GL_FALSE, glm::value_ptr(normal_matrix));

			// Render the mesh
			node->mesh->render();
		}

		// Recursively render children
		for (const auto& child : node->children) {
			renderOpaqueNodes(child, viewMatrix);
		}
	}

	void Scene::resize(unsigned width, unsigned height)
	{
		if (activeCamera) {
			activeCamera->aspect = float(width) / height;
		}
		glViewport(0, 0, width, height);

		GLenum error = glGetError(); 

		if (error != GL_NO_ERROR) 
		{ 
			std::cerr << "OpenGL error in resize: " << error << std::endl; 
		}
	}

	// Utility functions to manipulate scene
	std::shared_ptr<SceneNode> Scene::createNode(const std::string& name,
		std::shared_ptr<SceneNode> parent) {
		auto node = std::make_shared<SceneNode>(name);
		if (!parent) parent = root;
		parent->addChild(node);
		return node;
	}

	std::shared_ptr<SceneNode> Scene::findNode(const std::string& name,
		const std::shared_ptr<SceneNode>& startNode = nullptr) {
		auto node = startNode ? startNode : root;
		if (node->name == name) return node;

		for (const auto& child : node->children) {
			if (auto found = findNode(name, child)) {
				return found;
			}
		}
		return nullptr;
	}
	void Scene::handleKeyboard(const Uint8* keyboardState)
	{

		//Movement keys
		keyStates[SDL_SCANCODE_W] = keyboardState[SDL_SCANCODE_W];
		keyStates[SDL_SCANCODE_A] = keyboardState[SDL_SCANCODE_A];
		keyStates[SDL_SCANCODE_D] = keyboardState[SDL_SCANCODE_D];
		keyStates[SDL_SCANCODE_S] = keyboardState[SDL_SCANCODE_S];

		//Rotation keys
		keyStates[SDL_SCANCODE_UP] = keyboardState[SDL_SCANCODE_UP];
		keyStates[SDL_SCANCODE_DOWN] = keyboardState[SDL_SCANCODE_DOWN];
		keyStates[SDL_SCANCODE_LEFT] = keyboardState[SDL_SCANCODE_LEFT];
		keyStates[SDL_SCANCODE_RIGHT] = keyboardState[SDL_SCANCODE_RIGHT];

		// Reset key
		keyStates[SDL_SCANCODE_C] = keyboardState[SDL_SCANCODE_C];

		// Using static variables to detect key press/release for toggle actions
		static bool rWasPressed = false;
		static bool spaceWasPressed = false;

		// R - Toggle cube rotation on/off
		if (keyboardState[SDL_SCANCODE_R]) {
			if (!rWasPressed) {
				toggleCubeRotation();
				rWasPressed = true;

				// Visual feedback
				std::cout << "Cube rotation: "
					<< (cubeRotationSpeed != 0 ? "ACTIVE" : "PAUSED")
					<< std::endl;
			}
		}
		else {
			rWasPressed = false;
		}

		// Space - Reset cube to default orientation
		if (keyboardState[SDL_SCANCODE_SPACE]) {
			if (!spaceWasPressed) {
				cubeRotationAngle = 0.0f;
				if (transparentCubeNode) {
					transparentCubeNode->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
				}
				spaceWasPressed = true;
				std::cout << "Cube rotation reset to default" << std::endl;
			}
		}
		else {
			spaceWasPressed = false;
		}

		// Number keys for preset rotation speeds
		if (keyboardState[SDL_SCANCODE_1]) {
			cubeRotationSpeed = 0.5f;   // Slow rotation
			std::cout << "Cube rotation: SLOW (0.5 rad/s)" << std::endl;
		}
		if (keyboardState[SDL_SCANCODE_2]) {
			cubeRotationSpeed = 1.0f;   // Normal rotation
			std::cout << "Cube rotation: NORMAL (1.0 rad/s)" << std::endl;
		}
		if (keyboardState[SDL_SCANCODE_3]) {
			cubeRotationSpeed = 2.0f;   // Fast rotation
			std::cout << "Cube rotation: FAST (2.0 rad/s)" << std::endl;
		}
		if (keyboardState[SDL_SCANCODE_4]) {
			cubeRotationSpeed = -1.0f;  // Reverse rotation
			std::cout << "Cube rotation: REVERSE (-1.0 rad/s)" << std::endl;
		}
	}

	void Scene::toggleCubeRotation() {
		static float savedSpeed = 1.0f;
		if (cubeRotationSpeed != 0.0f) {
			savedSpeed = cubeRotationSpeed;
			cubeRotationSpeed = 0.0f;  // Stop rotation
		}
		else {
			cubeRotationSpeed = savedSpeed;  // Resume rotation
		}
	}

	void Scene::updateCamera(float deltaTime)
	{
		if (!activeCamera) return;

		// Handle camera reset
		if (keyStates[SDL_SCANCODE_C]) {
			resetCameraRotation();
			return;
		}

		//Get the camera's forward and right vectors from its rotation
		glm::mat4 rotationMatrix(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, activeCamera->rotation.y, glm::vec3(0, 1, 0));

		glm::vec3 forward = glm::vec3(
			-sin(activeCamera->rotation.y),
			0,
			-cos(activeCamera->rotation.y)
		);

		glm::vec3 right = glm::vec3(
			cos(activeCamera->rotation.y),
			0,
			-sin(activeCamera->rotation.y)
		);

		float moveSpeed = cameraSpeed * deltaTime;
		float turnSpeed = cameraRotationSpeed * deltaTime;

		if (keyStates[SDL_SCANCODE_W])
		{
			activeCamera->position += forward * moveSpeed;
		}
		if (keyStates[SDL_SCANCODE_S])
		{
			activeCamera->position -= forward * moveSpeed;
		}
		if (keyStates[SDL_SCANCODE_A])
		{
			activeCamera->position -= right * moveSpeed;
		}
		if (keyStates[SDL_SCANCODE_D])
		{
			activeCamera->position += right * moveSpeed;
		}

		// Handle arrow key rotation
		if (keyStates[SDL_SCANCODE_UP]) {
			activeCamera->rotation.x += turnSpeed;
		}
		if (keyStates[SDL_SCANCODE_DOWN]) {
			activeCamera->rotation.x -= turnSpeed;
		}
		if (keyStates[SDL_SCANCODE_LEFT]) {
			activeCamera->rotation.y += turnSpeed;
		}
		if (keyStates[SDL_SCANCODE_RIGHT]) {
			activeCamera->rotation.y -= turnSpeed;
		}

		// Clamp vertical rotation to prevent camera flipping
		activeCamera->rotation.x = glm::clamp(activeCamera->rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());
	}
	void Scene::resetCameraRotation()
	{
		if (activeCamera)
		{
			activeCamera->rotation = defaultCameraRotation;
		}
	}

	void Scene::updateTransparencyAnimation(float deltaTime)
	{
		static float animTime = 0.0f;
		animTime += deltaTime;

		// Create a breathing effect that's synchronized with rotation
		// This makes the cube appear to "pulse" as it spins
		float baseAlpha = 0.3f;
		float alphaVariation = 0.15f;

		// Use a different frequency than rotation to avoid visual monotony
		float alpha = baseAlpha + alphaVariation * sin(animTime * 1.5f);

		setTransparency(alpha);
	}
}

