
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
#include "SceneNode.hpp"
#include "Camera.hpp"

namespace space
{

    class Scene
    {

        std::unique_ptr<ShaderProgram> shader_program;
        std::shared_ptr<SceneNode> root;
        std::shared_ptr<Camera> activeCamera;

        GLuint model_view_matrix_id = -1;
        GLuint projection_matrix_id = -1;
        GLint normal_matrix_id = -1;

        float angle;


    public:
        
        Scene(unsigned width, unsigned height);

        void update();
        void render();
        void resize(unsigned width, unsigned height);
        void renderNode(const std::shared_ptr<SceneNode>& node, const glm::mat4& viewMatrix);
        std::shared_ptr<SceneNode> createNode(const std::string& name, std::shared_ptr<SceneNode> parent);
        std::shared_ptr<SceneNode> findNode(const std::string& name, const std::shared_ptr<SceneNode>& startNode);
    };
}