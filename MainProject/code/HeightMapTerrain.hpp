
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