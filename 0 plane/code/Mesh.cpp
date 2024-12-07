
#include "Mesh.hpp"

namespace space
{
	void Mesh::setUpMesh()
	{

		/**
		* Generate IDs for the VAO and VBOs.
		*/
		glGenVertexArrays(1, &vao_id);
		glGenBuffers(VBO_COUNT, vbo_ids);

		/**
		* Binding the vao array.
		*/
		glBindVertexArray(vao_id);

		/**
		* Vertices vbo
		*/
		glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[VERTICES_VBO]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		/**
		* Normals vbo
		*/
		glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[NORMALS_VBO]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);

		/**
		* Colors vbo
		*/
		glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[COLORS_VBO]);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);

		/**
		* Indices ebo
		*/
		glBindBuffer(GL_ARRAY_BUFFER, vbo_ids[INDICES_EBO]);
		glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		/**
		* Unbind vao
		*/
		glBindVertexArray(0);
	}
}