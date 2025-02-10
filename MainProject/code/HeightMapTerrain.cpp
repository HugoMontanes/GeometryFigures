
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

                //Generate color based on height
                glm::vec3 color;
                float normalizedHeight = yPos / heightScale; // Normalize height to 0-1 range

                if (normalizedHeight < 0.33f) {
                    // Blue to Green (0-33%)
                    float t = normalizedHeight / 0.5f;
                    color = glm::vec3(
                        0.0f,                    // Red
                        t,                       // Green
                        1.0f - (t * 0.5f)       // Blue
                    );
                }
                else if (normalizedHeight < 0.9f) {
                    // Green to White (33-66%)
                    float t = (normalizedHeight - 0.33f) / 0.33f;
                    color = glm::vec3(
                        t,                       // Red
                        1.0f,                   // Green
                        0.5f + (t * 0.5f)       // Blue
                    );
                }
                else {
                    // White (66-100%)
                    float t = (normalizedHeight - 0.66f) / 0.34f;
                    color = glm::vec3(
                        0.9f,                   // Red
                        0.9f,                   // Green
                        0.9f                   // Blue
                    );
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

                    normals[currentIndex] = glm::normalize(glm::cross(right - left, down - up));
                }
            }
        }

        // Calculate total number of indices needed
        size_t totalIndices = (width - 1) * (height - 1) * 6;  // 6 indices per quad (2 triangles)
        indices.reserve(totalIndices);

        for (int z = 0; z < height - 1; ++z)
        {
            for (int x = 0; x < width; ++x)
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