
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
    class HeightMapTerrain : public Mesh
    {
    private:

        int width;
        int height;
        float heightScale;
        std::string heightMapPath;


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

        // Get the height at a specific world position
        float getHeightAtWorldPosition(float worldX, float worldZ) const
        {
            // Convert world position to texture coordinates
            // The terrain spans from -10 to 10 in world space (20 units total)
            float u = (worldX + 10.0f) / 20.0f;
            float v = (worldZ + 10.0f) / 20.0f;

            // Clamp to valid range
            u = glm::clamp(u, 0.0f, 1.0f);
            v = glm::clamp(v, 0.0f, 1.0f);

            // Convert to texture pixel coordinates
            int x = static_cast<int>(u * (width - 1));
            int z = static_cast<int>(v * (height - 1));

            // For more accurate sampling, we could do bilinear interpolation
            // For now, we'll use nearest neighbor
            return getHeightAtGridPoint(x, z);
        }

        // Get normalized height (0-1 range) at world position
        float getNormalizedHeightAtWorldPosition(float worldX, float worldZ) const
        {
            float height = getHeightAtWorldPosition(worldX, worldZ);
            // Normalize based on the height scale used in terrain generation
            // The terrain Y ranges from 0 to 5*heightScale
            return height / (5.0f * heightScale);
        }

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
            const HeightMapTerrain& terrain,
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