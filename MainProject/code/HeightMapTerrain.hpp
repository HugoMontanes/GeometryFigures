/*
* Este código es de dominio público
* Realizado por Hugo Montañés García
*/

#pragma once

#include "Mesh.hpp"
#include "SceneNode.hpp"
#include "Scene.hpp"

#include <SOIL2.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>

namespace space
{
    class GrassMesh;

    struct GrassHeightInfo
    {
        float worldHeight;
        float normalizedHeight;
    };

    class HeightMapTerrain : public Mesh
    {
    private:

        int width;
        int height;
        float heightScale;
        std::string heightMapPath;

        // Store terrain parameters for grass generation
        float terrainWorldScale = 20.0f;

        //Converts RGB to grayscale
        float rgbToHeight(unsigned char r, unsigned char g, unsigned char b)
        {
            //Weighted grayscale conversion for more natural height
            return (0.299f * r + 0.587f * g + 0.114f * b) / 255.0f * heightScale;
        }

        // Get height at a specific grid point
        float getHeightAtGridPoint(int x, int z) const
        {
            // Clamp to valid range
            x = glm::clamp(x, 0, width - 1);
            z = glm::clamp(z, 0, height - 1);

            // Calculate vertex index
            int index = z * width + x;

            // Return the Y component of the vertex at this position
            if (index < vertices.size())
            {
                return vertices[index].y;
            }

            return 0.0f; // Default height
        }

    public:

        HeightMapTerrain(const std::string& path, float scale = 1.0f)
            : heightMapPath (path), heightScale(scale), width(0), height(0)
        {
            initialize();
        }

        void initialize() override;

        // Getter for terrain dimensions
        int getWidth() const { return width; }
        int getHeight() const { return height; }
        float getTerrainWorldScale() const { return terrainWorldScale; }
        float getHeightScale() const { return heightScale; }

        // Get the height at a specific world position
        float getHeightAtWorldPosition(float worldX, float worldZ, const glm::mat4& terrainTransform) const;

        float getHeightAtIndex(int index) const
        {
            // Bounds checking to prevent crashes
            if (index >= 0 && index < vertices.size())
            {
                return vertices[index].y;
            }

            // Return a default height if index is out of bounds
            return 0.0f;
        }

        std::shared_ptr<GrassMesh> createGrassForTerrain(
            const glm::mat4& terrainTransform,
            const std::string& grassModelPath,
            int instanceCount);

    };

    //Helper function to create a terrain node in the scene
    inline std::shared_ptr<SceneNode> createTerrainNode (
        Scene& scene, 
        const std::string& name, 
        const std::string& heightMapPath, 
        float heightScale = 1.0f, 
        const glm::vec3& position = glm::vec3(0.0f), 
        const glm::vec3& rotation = glm::vec3(0.0f), 
        const glm::vec3& scale = glm::vec3(1.0f))
    {
        // Create the terrain mesh
        auto terrainMesh = std::make_shared<HeightMapTerrain>(heightMapPath, heightScale);

        // Create a scene node for the terrain
        auto terrainNode = scene.createNode(name);
        terrainNode->mesh = terrainMesh;

        // Set position, rotation, and scale
        terrainNode->position = position;
        terrainNode->rotation = rotation;
        terrainNode->scale = scale;

        return terrainNode;
    }

}