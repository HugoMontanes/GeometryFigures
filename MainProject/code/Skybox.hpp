
#pragma once

#include "Mesh.hpp"
#include <glad/glad.h>
#include <string>
#include <vector>

namespace space
{
    class Skybox : public Mesh
    {
    private:
        GLuint textureID;

    public:
        Skybox(const std::vector<std::string>& facePaths);
        ~Skybox();

        void initialize() override;
        void render() override;

        GLuint getTextureID() const { return textureID; }
    };
}