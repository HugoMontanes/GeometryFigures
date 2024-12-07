
#pragma once

#include <glad/glad.h>
#include <vector>

namespace space
{
	class Mesh
	{
	protected:

		enum
		{
			VERTICES_VBO,
			NORMALS_VBO,
			COLORS_VBO,
			INDICES_EBO,
			VBO_COUNT
		};

		GLuint vbo_ids[VBO_COUNT];
		GLuint vao_id;

		std::vector<GLfloat> vertices;
		std::vector<GLfloat> normals;
		std::vector<GLfloat> colors;
		std::vector<GLuint> indices;

	public:

		/**
		* Makes shure al ids are initialized to 0.
		*/
		Mesh() : vao_id(0)
		{
			for (unsigned int& id : vbo_ids)
			{
				id = 0;
			}
		}

		virtual ~Mesh() { cleanUp(); }

		/**
		* Forces the children to implement and initialize the vertices, normals, color and indices.
		*/
		virtual void initialize() = 0;

		void setUpMesh();

		virtual void render() 
		{
			glBindVertexArray(vao_id);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}

		void cleanUp()
		{
			glDeleteVertexArrays(1, &vao_id);
			glDeleteBuffers(VBO_COUNT, vbo_ids);
		}

	};
}