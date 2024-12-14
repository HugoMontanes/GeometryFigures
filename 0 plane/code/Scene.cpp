
#pragma once

#include "Scene.hpp"

#include <iostream>
#include <cassert>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

namespace space
{

	Scene::Scene(unsigned width, unsigned height) 
	:	angle(0)
	{
		glEnable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);

		vertex_shader.loadFromFile("..\..\..\shared\assets\shaders\vertex");
		fragment_shader.loadFromFile("..\..\..\shared\assets\shaders\fragment");

		vertex_shader.
		
	}

	void Scene::update()
	{

	}

	void Scene::render()
	{

	}
}

