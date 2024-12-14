
#pragma once

#include"glm.hpp"

#include <memory>
#include <string>
#include <vector>

#include "Plane.hpp"
#include "ShaderProgram.hpp"
#include "VertexShader.hpp"
#include "FragmentShader.hpp"

namespace space
{

    class Scene
    {

        glm::mat4 model_view_matrix_id;
        glm::mat4 projection_matrix_id;
        GLint normal_matrix_id;

        float angle;

        VertexShader vertex_shader;
        FragmentShader fragment_shader;
        ShaderProgram shader_program;


    public:
        
        Scene(unsigned width, unsigned height);

        void update();
        void render();
        void resize(unsigned width, unsigned height);
    };
}