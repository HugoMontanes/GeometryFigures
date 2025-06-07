#pragma once

#include "Mesh.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
#include <random>
#include "HeightMapTerrain.hpp"

namespace space
{
	struct GrassInstance
	{
		glm::vec3 position;
		glm::vec3 color;
		float scale;
		float rotation;
	};

	class GrassMesh : public Mesh
	{
	private:

		std::unique_ptr<Assimp::Importer> importer;

		std::vector<GrassInstance> instances;
		GLuint instanceVBO;

		float minHeight = 0.2f; //Don't place grass below this normalized height (water level)
		float maxHeight = 0.7f; //Don't place grass above this normalized height (mountain level)
		float density = 0.5f; //Grass density (0-1)

		//Height-based colors (matching terrain color scheme)
		const glm::vec3 SHORE_GRASS_COLOR = glm::vec3(0.4f, 0.8f, 0.3f);
		const glm::vec3 MEADOW_GRASS_COLOR = glm::vec3(0.2f, 0.7f, 0.1f);
		const glm::vec3 HILL_GRASS_COLOR = glm::vec3(0.3f, 0.5f, 0.2f);

		void processMesh(aiMesh* mesh);

		glm::vec3 aiVec3ToGlm(const aiVector3D& vec) { return glm::vec3(vec.x, vec.y, vec.z); }

		glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t)
		{
			return a + t * (b - a);
		}

	public:
		GrassMesh() : instanceVBO(0)
		{
			importer = std::make_unique<Assimp::Importer>();
		}

		~GrassMesh()
		{
			if (instanceVBO)
			{
				glDeleteBuffers(1, &instanceVBO);
			}
		}

		//Load grass mesh from file using Assimp
		bool loadFromFile(const std::string& filepath);

		//Generate grass instances based on terrain heightmap
		void generateInstances(const HeightMapTerrain& terrain, int instanceCount);

		//Override initialize for manual mesh creation
		void initialize() override;

		// Override render to use instanced rendering
		void render() override;

		//Set up instance buffer after generating instances
		void setupInstanceBuffer();

		//Setters for grass parameters
		void setHeightRange(float min, float max) { minHeight = min; maxHeight = max; }
		void setDensity(float d) { density = glm::clamp(d, 0.0f, 1.0f); }

		//Get instance count
		size_t getInstanceCount() const { return instances.size(); }
	};
}

