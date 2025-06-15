/*
* Este código es de dominio público
* Realizado por Hugo Montañés García
*/

#pragma once

#include "Shader.hpp"

namespace space
{
	class FragmentShader : public Shader
	{

	public:
		FragmentShader() : Shader(GL_FRAGMENT_SHADER) {}

	};
}