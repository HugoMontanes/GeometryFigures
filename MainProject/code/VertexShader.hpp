/*
* Este c�digo es de dominio p�blico
* Realizado por Hugo Monta��s Garc�a
*/

#pragma once

#include "Shader.hpp"

namespace space
{
	class VertexShader : public Shader
	{

	public:

		VertexShader() : Shader(GL_VERTEX_SHADER) {}

	};
}