
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

			/**
			* Check for linkage errors
			*/
			GLint success;
			glGetProgramiv(program_id, GL_LINK_STATUS, &success);
			if (!success) {
				char info_log[512];
				glGetProgramInfoLog(program_id, 512, nullptr, info_log);
				std::cerr << "Shader program linking error:\n" << info_log << std::endl;
				return false;
			}
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