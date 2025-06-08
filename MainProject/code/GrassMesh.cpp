
#include "GrassMesh.hpp"
#include "HeightMapTerrain.hpp"
#include <iostream>
#include <functional>
#include <ext/scalar_constants.hpp>
#include <map>


namespace space
{
    bool GrassMesh::loadFromFile(const std::string& filepath)
    {
        // Load the mesh file using Assimp
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
            colors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
        }

        // Process indices
        indices.reserve(mesh->mNumFaces * 3);
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        std::cout << "Loaded grass mesh: " << vertices.size() << " vertices, "
            << indices.size() / 3 << " triangles" << std::endl;
    }

    glm::vec3 GrassMesh::getColorForHeight(float normalizedHeight)
    {
        // Determine grass color based on height
        // This matches the terrain coloring scheme
        glm::vec3 grassColor;

        if (normalizedHeight < 0.35f)
        {
            // Near shore - lighter, more vibrant grass
            grassColor = SHORE_GRASS_COLOR;
        }
        else if (normalizedHeight < 0.55f)
        {
            // Meadow - rich green grass
            float t = (normalizedHeight - 0.35f) / 0.2f;
            grassColor = lerp(SHORE_GRASS_COLOR, MEADOW_GRASS_COLOR, t);
        }
        else
        {
            // Hills - darker, more muted grass
            float t = (normalizedHeight - 0.55f) / 0.15f;
            grassColor = lerp(MEADOW_GRASS_COLOR, HILL_GRASS_COLOR, t);
        }

        return grassColor;
    }

    void GrassMesh::generateInstances(
        int instanceCount,
        float terrainWidth,
        float terrainHeight,
        std::function<float(float, float)> heightSampler)
    {
        instances.clear();
        instances.reserve(instanceCount);

        // Random number generation for placement
        std::random_device rd;
        std::mt19937 gen(rd());

        // Distribution for random placement across terrain
        std::uniform_real_distribution<float> posDistX(-terrainWidth / 2, terrainWidth / 2);
        std::uniform_real_distribution<float> posDistZ(-terrainHeight / 2, terrainHeight / 2);
        std::uniform_real_distribution<float> rotDist(0.0f, 2.0f * glm::pi<float>());
        std::uniform_real_distribution<float> scaleDist(0.8f, 1.2f);

        int attemptCount = 0;
        int maxAttempts = instanceCount * 10; // Allow multiple attempts

        while (instances.size() < instanceCount && attemptCount < maxAttempts)
        {
            attemptCount++;

            // Generate random position
            float x = posDistX(gen);
            float z = posDistZ(gen);

            // Sample height at this position using the provided function
            float normalizedHeight = heightSampler(x, z);

            // Check if this height is suitable for grass
            if (normalizedHeight < minHeight || normalizedHeight > maxHeight)
            {
                continue; // Skip positions that are too low (water) or too high (mountains)
            }

            // Calculate world Y position from normalized height
            // Assuming terrain height scale of 5.0f (matching your terrain)
            float worldY = normalizedHeight * 5.0f - 2.0f;

            // Get appropriate color for this height
            glm::vec3 grassColor = getColorForHeight(normalizedHeight);

            // Create grass instance
            GrassInstance instance;
            instance.position = glm::vec3(x, worldY, z);
            instance.color = grassColor;
            instance.scale = scaleDist(gen);
            instance.rotation = rotDist(gen);

            instances.push_back(instance);
        }

        std::cout << "Generated " << instances.size() << " grass instances" << std::endl;

        // Set up the instance buffer after generation
        setupInstanceBuffer();
    }

    void GrassMesh::generateInstancesForTerrain(int instanceCount, float worldWidth, float worldHeight, const glm::vec3& terrainWorldPos, std::function<GrassHeightInfo(float, float)> heightSampler)
    {
        instances.clear();
        instances.reserve(instanceCount);

        std::random_device rd;
        std::mt19937 gen(rd());

        // Distribution for random placement in world space
        // The terrain is centered at terrainWorldPos, so we distribute around it
        std::uniform_real_distribution<float> posDistX(
            terrainWorldPos.x - worldWidth / 2.0f,
            terrainWorldPos.x + worldWidth / 2.0f
        );
        std::uniform_real_distribution<float> posDistZ(
            terrainWorldPos.z - worldHeight / 2.0f,
            terrainWorldPos.z + worldHeight / 2.0f
        );

        // Random distributions for visual variety
        std::uniform_real_distribution<float> rotDist(0.0f, 2.0f * glm::pi<float>());
        std::uniform_real_distribution<float> scaleDist(0.001f, 0.003f);
        std::uniform_real_distribution<float> heightOffsetDist(-0.05f, 0.05f);
        std::uniform_real_distribution<float> colorVariation(0.9f, 1.1f);

        int attemptCount = 0;
        int maxAttempts = instanceCount * 10;  // Allow multiple attempts for better coverage

        // Debug info
        std::cout << "Generating grass in world bounds:" << std::endl;
        std::cout << "  X: [" << (terrainWorldPos.x - worldWidth / 2)
            << " to " << (terrainWorldPos.x + worldWidth / 2) << "]" << std::endl;
        std::cout << "  Z: [" << (terrainWorldPos.z - worldHeight / 2)
            << " to " << (terrainWorldPos.z + worldHeight / 2) << "]" << std::endl;

        while (instances.size() < instanceCount && attemptCount < maxAttempts)
        {
            attemptCount++;

            // Generate random position in world space
            float worldX = posDistX(gen);
            float worldZ = posDistZ(gen);

            // Sample height information at this position
            GrassHeightInfo heightInfo = heightSampler(worldX, worldZ);

            // Check if this height is suitable for grass
            // normalizedHeight should be between 0 and 1
            if (heightInfo.normalizedHeight < minHeight || heightInfo.normalizedHeight > maxHeight)
            {
                continue;  // Skip water and mountain peaks
            }

            // Use the world height directly from the sampler
            float worldY = heightInfo.worldHeight;

            // Add slight random height offset for natural variation
            worldY += heightOffsetDist(gen);

            // Get appropriate color for this height
            glm::vec3 grassColor = getColorForHeight(heightInfo.normalizedHeight);

            // Add some color variation for natural look
            grassColor *= colorVariation(gen);
            grassColor = glm::clamp(grassColor, glm::vec3(0.0f), glm::vec3(1.0f));

            // Create grass instance
            GrassInstance instance;
            instance.position = glm::vec3(worldX, worldY, worldZ);
            instance.color = grassColor;
            instance.scale = scaleDist(gen);
            instance.rotation = rotDist(gen);

            instances.push_back(instance);
        }

        std::cout << "Generated " << instances.size() << " grass instances out of "
            << instanceCount << " requested (attempt ratio: "
            << (float)attemptCount / instances.size() << ")" << std::endl;

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

        // Pack instance data: position (3), color (3), scale (1), rotation (1) = 8 floats
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

        // Use instanced drawing - this is much more efficient than drawing each instance separately
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

    void GrassMesh::printStatistics() const
    {
        if (instances.empty())
        {
            std::cout << "No grass instances generated!" << std::endl;
            return;
        }

        // Calculate bounds and statistics
        glm::vec3 minPos = instances[0].position;
        glm::vec3 maxPos = instances[0].position;
        float avgHeight = 0.0f;

        std::map<std::string, int> colorZones;

        for (const auto& instance : instances)
        {
            minPos = glm::min(minPos, instance.position);
            maxPos = glm::max(maxPos, instance.position);
            avgHeight += instance.position.y;

            // Categorize by color (height zone)
            std::string zone = "unknown";
            if (instance.color.g > 0.7f && instance.color.r < 0.3f)
                zone = "meadow";
            else if (instance.color.r > 0.35f && instance.color.g > 0.7f)
                zone = "shore";
            else if (instance.color.g < 0.6f)
                zone = "hill";

            colorZones[zone]++;
        }

        avgHeight /= instances.size();

        std::cout << "\n=== Grass Statistics ===" << std::endl;
        std::cout << "Total instances: " << instances.size() << std::endl;
        std::cout << "Bounds: " << std::endl;
        std::cout << "  X: [" << minPos.x << " to " << maxPos.x
            << "] (span: " << (maxPos.x - minPos.x) << ")" << std::endl;
        std::cout << "  Y: [" << minPos.y << " to " << maxPos.y
            << "] (span: " << (maxPos.y - minPos.y) << ")" << std::endl;
        std::cout << "  Z: [" << minPos.z << " to " << maxPos.z
            << "] (span: " << (maxPos.z - minPos.z) << ")" << std::endl;
        std::cout << "Average height: " << avgHeight << std::endl;
        std::cout << "Distribution by zone:" << std::endl;
        for (const auto& [zone, count] : colorZones)
        {
            std::cout << "  " << zone << ": " << count
                << " (" << (100.0f * count / instances.size()) << "%)" << std::endl;
        }
        std::cout << "========================\n" << std::endl;
    }
}