/*
* Este código es de dominio público
* Realizado por Hugo Montañés García
*/

#include "Skybox.hpp"
#include <SOIL2.h>
#include <iostream>

namespace space
{
    Skybox::Skybox(const std::vector<std::string>& facePaths)
    {
        // Generate and bind cubemap texture
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        // Load each face of the cubemap
        int width, height, channels;
        for (unsigned int i = 0; i < facePaths.size(); i++)
        {
            unsigned char* data = SOIL_load_image(
                facePaths[i].c_str(),
                &width, &height,
                &channels,
                SOIL_LOAD_RGB
            );

            if (data)
            {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
                SOIL_free_image_data(data);
            }
            else
            {
                std::cerr << "Cubemap texture failed to load at path: " << facePaths[i] << std::endl;
                SOIL_free_image_data(data);
            }
        }

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // Initialize the skybox mesh
        initialize();
    }

    Skybox::~Skybox()
    {
        glDeleteTextures(1, &textureID);
    }

    void Skybox::initialize()
    {
        // Create a unit cube centered at origin
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        // Convert to format used by your engine
        for (int i = 0; i < 36; i++) {
            vertices.push_back(glm::vec3(
                skyboxVertices[i * 3],
                skyboxVertices[i * 3 + 1],
                skyboxVertices[i * 3 + 2]
            ));

            // Since we're using the position as texture coordinates in the shader,
            // we don't need specific texture coords. Add placeholder normals and colors.
            normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
            colors.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
            indices.push_back(i);
        }

        // Set up mesh buffers
        setUpMesh();
    }

    void Skybox::render()
    {
        // Save current OpenGL state
        GLboolean depthTest;
        glGetBooleanv(GL_DEPTH_TEST, &depthTest);

        // Disable depth writing so skybox is always in background
        glDepthFunc(GL_LEQUAL);

        // Bind the cubemap texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        // Render the mesh
        Mesh::render();

        // Restore previous OpenGL state
        if (depthTest)
            glDepthFunc(GL_LESS);
    }
}