
#include"HeightMapTerrain.hpp"

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
}