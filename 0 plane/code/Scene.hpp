
#pragma once

#include "Mesh.hpp"
#include "Plane.hpp"
#include "ShaderProgram.hpp"

#include"glm.hpp"

#include <memory>
#include <string>
#include <vector>

namespace space
{

    class Scene
    {

        glm::mat4 model_view_matrix_id;
        glm::mat4 projection_matrix_id;
        GLint normal_matrix_id;



    public:
        
        Scene(unsigned width, unsigned height);

        void update();
        void render();
        void resize(unsigned width, unsigned height);
    };
}