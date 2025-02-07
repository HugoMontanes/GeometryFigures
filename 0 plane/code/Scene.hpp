
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
#include "Cone.hpp"
#include "ShaderProgram.hpp"
#include "VertexShader.hpp"
#include "FragmentShader.hpp"

namespace space
{
    struct Transform
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
    };

    class Scene
    {

        GLuint model_view_matrix_id;
        GLuint projection_matrix_id;
        GLint normal_matrix_id;

        float angle;

        std::unique_ptr<ShaderProgram> shader_program;
        std::vector<std::pair<std::shared_ptr<Mesh>, Transform>> scene_objects;

    public:
        
        Scene(unsigned width, unsigned height);

        void update();
        void render();
        void resize(unsigned width, unsigned height);
    };
}