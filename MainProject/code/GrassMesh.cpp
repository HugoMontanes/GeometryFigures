#include "GrassMesh.hpp"
#include <iostream>

namespace space
{
    bool GrassMesh::loadFromFile(const std::string& filepath)
    {
        // Load the mesh file using Assimp
        // aiProcess_Triangulate ensures all faces are triangles
        // aiProcess_GenNormals generates normals if they're missing
        const aiScene* scene = importer->ReadFile(filepath,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace
        );

        // Check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cerr << "Assimp Error: " << importer->GetErrorString() << std::endl;
            return false;
        }

        // For grass, we typically just need the first mesh
        // More complex models might have multiple meshes
        if (scene->mNumMeshes > 0)
        {
            processMesh(scene->mMeshes[0]);
            setUpMesh(); // Call the parent class method to set up VAO/VBOs
            return true;
        }

        return false;
    }

    void GrassMesh::processMesh(aiMesh* mesh)
    {
        // Clear any existing data
        vertices.clear();
        normals.clear();
        colors.clear();
        indices.clear();

        // Reserve space for efficiency
        vertices.reserve(mesh->mNumVertices);
        normals.reserve(mesh->mNumVertices);
        colors.reserve(mesh->mNumVertices);

        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            // Position
            vertices.push_back(aiVec3ToGlm(mesh->mVertices[i]));

            // Normal
            if (mesh->HasNormals())
            {
                normals.push_back(aiVec3ToGlm(mesh->mNormals[i]));
            }
            else
            {
                normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f)); // Default up normal
            }

            // Color - will be overridden by instance color
            // Using white as base color to be modulated by instance color
            colors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
        }

        // Process indices
        indices.reserve(mesh->mNumFaces * 3); // Assuming triangles
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // We triangulated the mesh, so each face should have 3 vertices
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        std::cout << "Loaded grass mesh: " << vertices.size() << " vertices, "
            << indices.size() / 3 << " triangles" << std::endl;
    }

    void GrassMesh::generateInstances(const HeightMapTerrain& terrain, int instanceCount)
    {
        instances.clear();
        instances.reserve(instanceCount);

        // Random number generation for placement
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDistX(-10.0f, 10.0f); // Terrain spans -10 to 10
        std::uniform_real_distribution<float> posDistZ(-10.0f, 10.0f);
        std::uniform_real_distribution<float> rotDist(0.0f, 2.0f * glm::pi<float>());
        std::uniform_real_distribution<float> scaleDist(0.8f, 1.2f); // Slight size variation

        int attemptCount = 0;
        int maxAttempts = instanceCount * 10; // Allow multiple attempts to place grass

        while (instances.size() < instanceCount && attemptCount < maxAttempts)
        {
            attemptCount++;

            // Generate random position
            float x = posDistX(gen);
            float z = posDistZ(gen);

            // Sample height from terrain at this position
            // Convert world position to terrain texture coordinates
            float u = (x + 10.0f) / 20.0f; // Map from [-10,10] to [0,1]
            float v = (z + 10.0f) / 20.0f;

            // Get terrain dimensions and sample height
            int terrainWidth = terrain.getWidth();
            int terrainHeight = terrain.getHeight();

            int texX = static_cast<int>(u * (terrainWidth - 1));
            int texZ = static_cast<int>(v * (terrainHeight - 1));

            // Clamp to valid range
            texX = glm::clamp(texX, 0, terrainWidth - 1);
            texZ = glm::clamp(texZ, 0, terrainHeight - 1);

            // Get height from terrain vertex data
            // The terrain generates vertices in a grid, so we can calculate the index
            int vertexIndex = texZ * terrainWidth + texX;

            // This is a simplified height sampling - in practice, you might want to
            // interpolate between vertices for smoother placement
            float terrainY = 0.0f; // Default height

            // For now, we'll use a simplified height calculation based on the normalized position
            // In a full implementation, you'd access the actual terrain vertex data
            float normalizedHeight = 0.4f + 0.2f * sin(x * 0.5f) * cos(z * 0.5f); // Placeholder

            // Check if this height is suitable for grass
            if (normalizedHeight < minHeight || normalizedHeight > maxHeight)
            {
                continue; // Skip this position
            }

            // Determine grass color based on height
            glm::vec3 grassColor;
            if (normalizedHeight < 0.35f)
            {
                // Near shore
                grassColor = SHORE_GRASS_COLOR;
            }
            else if (normalizedHeight < 0.55f)
            {
                // Meadow
                float t = (normalizedHeight - 0.35f) / 0.2f;
                grassColor = lerp(SHORE_GRASS_COLOR, MEADOW_GRASS_COLOR, t);
            }
            else
            {
                // Hills
                float t = (normalizedHeight - 0.55f) / 0.15f;
                grassColor = lerp(MEADOW_GRASS_COLOR, HILL_GRASS_COLOR, t);
            }

            // Create grass instance
            GrassInstance instance;
            instance.position = glm::vec3(x, normalizedHeight * 5.0f - 2.0f, z); // Match terrain Y scaling
            instance.color = grassColor;
            instance.scale = scaleDist(gen);
            instance.rotation = rotDist(gen);

            instances.push_back(instance);
        }

        std::cout << "Generated " << instances.size() << " grass instances" << std::endl;

        // Set up the instance buffer after generation
        setupInstanceBuffer();
    }

    void GrassMesh::setupInstanceBuffer()
    {
        if (instances.empty()) return;

        // Delete old buffer if it exists
        if (instanceVBO)
        {
            glDeleteBuffers(1, &instanceVBO);
        }

        // Generate new buffer
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        // We'll pack instance data: position (3), color (3), scale (1), rotation (1) = 8 floats per instance
        std::vector<float> instanceData;
        instanceData.reserve(instances.size() * 8);

        for (const auto& inst : instances)
        {
            // Position
            instanceData.push_back(inst.position.x);
            instanceData.push_back(inst.position.y);
            instanceData.push_back(inst.position.z);
            // Color
            instanceData.push_back(inst.color.x);
            instanceData.push_back(inst.color.y);
            instanceData.push_back(inst.color.z);
            // Scale and rotation
            instanceData.push_back(inst.scale);
            instanceData.push_back(inst.rotation);
        }

        // Upload instance data
        glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(float),
            instanceData.data(), GL_STATIC_DRAW);

        // Bind VAO to set up instance attributes
        glBindVertexArray(vao_id);

        // Instance position (location = 3)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glVertexAttribDivisor(3, 1); // This attribute advances once per instance

        // Instance color (location = 4)
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glVertexAttribDivisor(4, 1);

        // Instance scale (location = 5)
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glVertexAttribDivisor(5, 1);

        // Instance rotation (location = 6)
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void GrassMesh::render()
    {
        if (vao_id == 0 || instances.empty())
        {
            std::cerr << "Error: Grass VAO not initialized or no instances." << std::endl;
            return;
        }

        glBindVertexArray(vao_id);

        // Use instanced drawing
        glDrawElementsInstanced(GL_TRIANGLES,
            static_cast<GLsizei>(indices.size()),
            GL_UNSIGNED_INT,
            nullptr,
            static_cast<GLsizei>(instances.size()));

        glBindVertexArray(0);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "OpenGL error in grass render: " << error << std::endl;
        }
    }
}