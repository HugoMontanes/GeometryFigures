/*
* Este código es de dominio público
* Realizado por Hugo Montañés García
*/

#pragma once

#include "Mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
#include <random>
#include <functional>

namespace space
{
    // Forward declaration from HeightMapTerrain
    struct GrassHeightInfo;

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

        // Grass parameters
        float minHeight = 0.15f;
        float maxHeight = 0.7f;
        float density = 0.5f;

        // Color constants
        const glm::vec3 SHORE_GRASS_COLOR = glm::vec3(0.4f, 0.8f, 0.3f);
        const glm::vec3 MEADOW_GRASS_COLOR = glm::vec3(0.2f, 0.7f, 0.1f);
        const glm::vec3 HILL_GRASS_COLOR = glm::vec3(0.3f, 0.5f, 0.2f);

        void processMesh(aiMesh* mesh);
        glm::vec3 aiVec3ToGlm(const aiVector3D& vec) { return glm::vec3(vec.x, vec.y, vec.z); }
        glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float t) { return a + t * (b - a); }

        // New method: determine grass color based on height
        glm::vec3 getColorForHeight(float normalizedHeight);

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

        bool loadFromFile(const std::string& filepath);

        void generateInstances(
            int instanceCount,
            float terrainWidth,
            float terrainHeight,
            std::function<float(float, float)> heightSampler
        );

        void generateInstancesForTerrain(
            int instanceCount,
            float worldWidth,
            float worldHeight,
            const glm::vec3& terrainWorldPos,
            std::function<GrassHeightInfo(float, float)> heightSampler
        );

        void initialize() override {}
        void render() override;
        void setupInstanceBuffer();

        void setHeightRange(float min, float max) { minHeight = min; maxHeight = max; }
        void setDensity(float d) { density = glm::clamp(d, 0.0f, 1.0f); }
        size_t getInstanceCount() const { return instances.size(); }

        void printStatistics() const;
    };
}