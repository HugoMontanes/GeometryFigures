/*
* Este c�digo es de dominio p�blico
* Realizado por Hugo Monta��s Garc�a
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