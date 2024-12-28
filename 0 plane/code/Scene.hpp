
#pragma once

#include"glm.hpp"
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <memory>
#include <string>
#include <vector>

#include "Mesh.hpp"
#include "Plane.hpp"
#include "ShaderProgram.hpp"
#include "VertexShader.hpp"
#include "FragmentShader.hpp"

namespace space
{

    class Scene
    {

        GLuint model_view_matrix_id;
        GLuint projection_matrix_id;
        GLint normal_matrix_id;

        float angle;

        std::unique_ptr<ShaderProgram> shader_program;
        std::vector<std::shared_ptr<Mesh>>meshes;

    public:
        
        Scene(unsigned width, unsigned height);

        void update();
        void render();
        void resize(unsigned width, unsigned height);
    };
}