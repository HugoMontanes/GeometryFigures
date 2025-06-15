/*
* Este código es de dominio público
* Realizado por Hugo Montañés García
*/

#include "HeightMapTerrain.hpp"

namespace space
{
    void HeightMapTerrain::initialize()
    {
        //Load height map image
        int channels;
        unsigned char* image = SOIL_load_image(
            heightMapPath.c_str(),
            &width, &height,
            &channels,
            SOIL_LOAD_RGB
        );

        if (!image)
        {
            std::cerr << "Failed to load height map: " << heightMapPath << std::endl;
        }

        // Pre-allocate memory for vectors
        size_t totalVertices = width * height;
        vertices.reserve(totalVertices);
        colors.reserve(totalVertices);
        normals.reserve(totalVertices);

        // Define base colors for different heights
        const glm::vec3 WATER_COLOR(0.0f, 0.3f, 1.0f);    // Deep blue for lowest areas
        const glm::vec3 SHORE_COLOR(0.0f, 0.8f, 0.8f);    // Teal/light blue for shallow water
        const glm::vec3 GRASS_COLOR(0.0f, 0.7f, 0.0f);    // Green for mid elevations
        const glm::vec3 MOUNTAIN_COLOR(0.5f, 0.5f, 0.5f); // Gray for higher elevations
        const glm::vec3 SNOW_COLOR(0.9f, 0.9f, 0.9f);     // White for peaks

        // Linear interpolation helper function (can be added as a utility or inline)
        auto lerp = [](const glm::vec3& a, const glm::vec3& b, float t) -> glm::vec3 {
            return a + t * (b - a);
            };

        //Generate vertices
        for (int z = 0; z < height; z++)
        {
            for (int x = 0; x < width; x++)
            {
                //Get pixel data
                int index = (x + z * width) * 3;

                unsigned char r = image[index];

                unsigned char g = image[index + 1];

                unsigned char b = image[index + 2];


                //Normalize coordinates and scale terrain
                float xPos = ((float)x / (width - 1) - 0.5f) * 20.0f;
                float zPos = ((float)z / (height - 1) - 0.5f) * 20.0f;

                //Height based on pixel intensity
                float yPos = rgbToHeight(r, g, b) * 5.0f;

                //Add vertex
                vertices.push_back(glm::vec3(xPos, yPos, zPos));

                // Generate color based on normalized height
                float normalizedHeight = yPos / (5.0f * heightScale); // Properly normalize to 0-1
                glm::vec3 color;

                if (normalizedHeight < 0.2f) {
                    // Water: Deep blue to shore blue (0-20%)
                    float t = normalizedHeight / 0.2f;
                    color = lerp(WATER_COLOR, SHORE_COLOR, t);
                }
                else if (normalizedHeight < 0.4f) {
                    // Shore to grass (20-40%)
                    float t = (normalizedHeight - 0.2f) / 0.2f;
                    color = lerp(SHORE_COLOR, GRASS_COLOR, t);
                }
                else if (normalizedHeight < 0.7f) {
                    // Grass to mountain (40-70%)
                    float t = (normalizedHeight - 0.4f) / 0.3f;
                    color = lerp(GRASS_COLOR, MOUNTAIN_COLOR, t);
                }
                else {
                    // Mountain to snow (70-100%)
                    float t = (normalizedHeight - 0.7f) / 0.3f;
                    color = lerp(MOUNTAIN_COLOR, SNOW_COLOR, t);
                }

                colors.push_back(color);

                //Generate normals
                glm::vec3 normal(0.0f, 1.0f, 0.0f);

                normals.push_back(normal);
            }
        }

        // Second pass: Calculate normals
        for (int z = 0; z < height; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                if (x > 0 && z > 0 && x < width - 1 && z < height - 1)
                {
                    size_t currentIndex = z * width + x;
                    size_t leftIndex = z * width + (x - 1);
                    size_t rightIndex = z * width + (x + 1);
                    size_t upIndex = (z - 1) * width + x;
                    size_t downIndex = (z + 1) * width + x;

                    glm::vec3& left = vertices[leftIndex];
                    glm::vec3& right = vertices[rightIndex];
                    glm::vec3& up = vertices[upIndex];
                    glm::vec3& down = vertices[downIndex];

                    normals[currentIndex] = glm::normalize(glm::cross(down - up, right - left));
                }
            }
        }

        // Calculate total number of indices needed
        size_t totalIndices = (width - 1) * (height - 1) * 6;  // 6 indices per quad (2 triangles)
        indices.reserve(totalIndices);

        for (int z = 0; z < height - 1; ++z)
        {
            for (int x = 0; x < width - 1; ++x)
            {
                GLuint topLeft = z * width + x;
                GLuint topRight = z * width + x + 1;
                GLuint bottomLeft = (z + 1) * width + x;
                GLuint bottomRight = (z + 1) * width + x + 1;

                // First triangle
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // Second triangle
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        // Free the image
        SOIL_free_image_data(image);

        // Set up OpenGL buffers
        setUpMesh();
    }

    float HeightMapTerrain::getHeightAtWorldPosition(float worldX, float worldZ, const glm::mat4& terrainTransform) const
    {
        // Transform world position to terrain's local space
        glm::vec4 worldPos(worldX, 0, worldZ, 1);
        glm::mat4 invTransform = glm::inverse(terrainTransform);
        glm::vec4 localPos = invTransform * worldPos;

        // Now localPos is in the terrain's coordinate system (-10 to 10)
        // Convert to texture coordinates (0 to 1)
        float u = (localPos.x + terrainWorldScale * 0.5f) / terrainWorldScale;
        float v = (localPos.z + terrainWorldScale * 0.5f) / terrainWorldScale;

        // Clamp to valid range
        u = glm::clamp(u, 0.0f, 1.0f);
        v = glm::clamp(v, 0.0f, 1.0f);

        // Get the four surrounding vertices for bilinear interpolation
        float fx = u * (width - 1);
        float fz = v * (height - 1);

        int x0 = (int)fx;
        int z0 = (int)fz;
        int x1 = glm::min(x0 + 1, width - 1);
        int z1 = glm::min(z0 + 1, height - 1);

        // Get the fractional parts for interpolation
        float wx = fx - x0;
        float wz = fz - z0;

        // Get heights at the four corners
        float h00 = getHeightAtIndex(z0 * width + x0);
        float h10 = getHeightAtIndex(z0 * width + x1);
        float h01 = getHeightAtIndex(z1 * width + x0);
        float h11 = getHeightAtIndex(z1 * width + x1);

        // Bilinear interpolation
        float h0 = h00 * (1 - wx) + h10 * wx;
        float h1 = h01 * (1 - wx) + h11 * wx;
        float height = h0 * (1 - wz) + h1 * wz;

        // Apply the terrain's Y scale to the height
        return height * terrainTransform[1][1];
    }

    std::shared_ptr<GrassMesh> HeightMapTerrain::createGrassForTerrain(
        const glm::mat4& terrainTransform,
        const std::string& grassModelPath,
        int instanceCount)
    {
        // Create grass mesh
        auto grass = std::make_shared<GrassMesh>();

        // Load the grass model
        if (!grass->loadFromFile(grassModelPath))
        {
            std::cerr << "Failed to load grass model: " << grassModelPath << std::endl;
            return nullptr;
        }

        // Create a height sampling function that properly handles coordinate transforms
        auto heightSampler = [this, terrainTransform](float worldX, float worldZ) -> GrassHeightInfo {
            // Get the actual height at this world position
            float localHeight = getHeightAtWorldPosition(worldX, worldZ, terrainTransform);

            // Get terrain's world Y position from transform
            float terrainWorldY = terrainTransform[3][1];

            // Calculate absolute world height
            float absoluteWorldHeight = terrainWorldY + localHeight;

            // Normalize based on expected height range
            // The terrain heights range from 0 to 5*heightScale in local space
            // After transform and offset, they range from (terrainY) to (terrainY + 5*scale*heightScale)
            float maxHeightRange = 5.0f * heightScale * terrainTransform[1][1];
            float normalizedHeight = localHeight / (5.0f * heightScale);

            return GrassHeightInfo{ absoluteWorldHeight, normalizedHeight };
            };

        // Calculate the world space bounds of the terrain
        float worldScale = terrainTransform[0][0] * terrainWorldScale;  // X scale * terrain size

        // Get terrain world position
        glm::vec3 terrainWorldPos(terrainTransform[3][0], terrainTransform[3][1], terrainTransform[3][2]);

        // Generate instances with proper world bounds
        grass->generateInstancesForTerrain(
            instanceCount,
            worldScale,        // Actual world width
            worldScale,        // Actual world height  
            terrainWorldPos,   // Terrain center in world space
            heightSampler
        );

        return grass;
    }
}