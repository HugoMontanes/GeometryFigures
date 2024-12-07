
#pragma once

#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <string>

#include "Shader.hpp"

namespace space
{
	class ShaderProgram
	{
	private:

		GLuint program_id;

	public:

		ShaderProgram() : program_id(glCreateProgram()) {}

		~ShaderProgram()
		{
			if (program_id)
			{
				glDeleteProgram(program_id);
			}
		}

		void attachShader(const Shader& shader)
		{
			glAttachShader(program_id, shader.getShaderID());
		}

		bool link() 
		{
			glLinkProgram(program_id);
		}

		void use()
		{
			glUseProgram(program_id);
		}

		GLuint getProgramID() const
		{
			return program_id;
		}
	};
}