
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

		auto terrainNode = space::createTerrainNode(
			*this,                          // Scene reference
			"main_terrain",                 // Node name
			"../../../shared/assets/textures/heightmaps/heightmap_010.png",      // Path to height map
			1.0f,                           // Height scale
			glm::vec3(0, -2, 0),            // Position
			glm::vec3(0, 0, 0),             // Rotation
			glm::vec3(10.0f)                 // Scale
		);

		/*auto planeNode = std::make_shared<SceneNode>("plane");
		planeNode->mesh = std::make_shared<Plane>(5, 5, 10.0f, 10.0f);
		planeNode->position = glm::vec3(0, -2, 0);
		root->addChild(planeNode);

		auto coneNode = std::make_shared<SceneNode>("cone");
		coneNode->mesh = std::make_shared<Cone>(100);
		coneNode->position = glm::vec3(0, 1, 0);
		planeNode->addChild(coneNode);*/

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
		angle += 0.01f;

		//Get current keyboard state
		const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
		handleKeyboard(keyboardState);

		updateCamera(deltaTime);
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

		// First render the skybox
		skybox_shader->use();

		// Remove translation from the view matrix to keep skybox centered around camera
		glm::mat4 skybox_view = glm::mat4(glm::mat3(view_matrix));

		// Send matrices to shader
		glUniformMatrix4fv(skybox_view_matrix_id, 1, GL_FALSE, glm::value_ptr(skybox_view));
		glUniformMatrix4fv(skybox_projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		// Render skybox
		skybox->render();

		// Now render the rest of the scene
		shader_program->use();
		glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		renderNode(root, view_matrix);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "OpenGL error in render: " << error << std::endl;
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
}

