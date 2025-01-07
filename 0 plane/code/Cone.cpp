
#include "Cone.hpp"
#include <ext/scalar_constants.hpp>


namespace space
{
	void Cone::initialize()
	{
		vertices.clear(); 
		normals.clear(); 
		colors.clear(); 
		indices.clear();

		vertices.reserve(slices + 2);
		normals.reserve(slices + 2);
		colors.reserve(slices + 2);
		indices.reserve(slices * 3);
		/**
		* The top vertex
		*/
		vertices.emplace_back(0.0f, height, 0.0f);
		normals.emplace_back(0.0f, 1.0f, 0.0f);
		colors.emplace_back(1.0f, 0.0f, 0.0f);

		/**
		* The bottom center vertex
		*/
		vertices.emplace_back(0.0f, 0.0f, 0.0f); 
		normals.emplace_back(0.0f, -1.0f, 0.0f); 
		colors.emplace_back(1.0f, 0.0f, 0.0f);
		/**
		* The bottom vertices
		*/
		for (unsigned int i = 0; i < slices; ++i)
		{
			float angle = 2.0f * glm::pi<float>() * i / slices;
			float x = radius * std::cos(angle);
			float z = radius * std::sin(angle);
			vertices.emplace_back(x, 0.0f, z);
			normals.emplace_back(0.0f, -1.0f, 0.0f);
			colors.emplace_back(1.0f, 0.0f, 0.0f);
		}
		/**
		* The indices
		*/

		 for (unsigned int i = 1; i <= slices; ++i) 
		 { 
			indices.push_back(0); 
			indices.push_back(i + 1); 
			indices.push_back((i % slices) + 2); 
		 } 

		 // The indices for the cone's base 
		 for (unsigned int i = 1; i <= slices; ++i) 
		 { 
			indices.push_back(1); 
			indices.push_back((i % slices) + 2); 
			indices.push_back(i + 1); 
		 }
	}
}