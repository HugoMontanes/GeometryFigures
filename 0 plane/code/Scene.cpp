
#include "Scene.hpp"

#include <iostream>
#include <cassert>


namespace space
{

	Scene::Scene(unsigned width, unsigned height)
		: angle(0.0f)
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
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

		auto plane = std::make_shared<Plane>(5, 5, 10.0f, 10.0f);
		meshes.push_back(plane);

		model_view_matrix_id = glGetUniformLocation(shader_program->getProgramID(), "model_view_matrix");
		normal_matrix_id = glGetUniformLocation(shader_program->getProgramID(), "normal_matrix");
		projection_matrix_id = glGetUniformLocation(shader_program->getProgramID(), "projection_matrix");

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "OpenGL error in Scene constructor: " << error << std::endl;
		}
	}
	void Scene::update()
	{
		angle += 0.01f;
	}

	void Scene::render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_program->use();

		glm::mat4 model_view_matrix(1);

		model_view_matrix = glm::translate(model_view_matrix, glm::vec3(0.f, 0.f, -4.f));
		model_view_matrix = glm::rotate(model_view_matrix, angle, glm::vec3(1.f, 2.f, 1.f));

		glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(model_view_matrix));

		glm::mat4 normal_matrix = glm::transpose(glm::inverse(model_view_matrix));

		glUniformMatrix4fv(normal_matrix_id, 1, GL_FALSE, glm::value_ptr(normal_matrix));

		for (const auto& mesh : meshes) 
		{ 
			mesh->render(); 
		}

		GLenum error = glGetError(); 
		if (error != GL_NO_ERROR) 
		{ 
			std::cerr << "OpenGL error in render: " << error << std::endl; 
		}
	}

	void Scene::resize(unsigned width, unsigned height)
	{
		glm::mat4 projection_matrix = glm::perspective(20.f, GLfloat(width) / height, 1.f, 500.f);

		glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));

		glViewport(0, 0, width, height);

		GLenum error = glGetError(); 

		if (error != GL_NO_ERROR) 
		{ 
			std::cerr << "OpenGL error in resize: " << error << std::endl; 
		}
	}
}

