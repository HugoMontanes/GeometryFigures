
#pragma once

#include"glm.hpp"
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <SDL.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "Mesh.hpp"
#include "Plane.hpp"
#include "Cone.hpp"
#include "ShaderProgram.hpp"
#include "VertexShader.hpp"
#include "FragmentShader.hpp"
#include "SceneNode.hpp"
#include "Camera.hpp"
#include "Skybox.hpp"
#include "GrassMesh.hpp"

namespace space
{

    class Scene
    {

        std::unique_ptr<ShaderProgram> shader_program;
        std::shared_ptr<SceneNode> root;
        std::shared_ptr<Camera> activeCamera;

        float cameraSpeed = 10.0f;
        float cameraRotationSpeed = 2.0f;
        glm::vec3 defaultCameraRotation = glm::vec3(0.0f); // Store default rotation
        std::unordered_map<SDL_Scancode, bool> keyStates;

        GLuint model_view_matrix_id = -1;
        GLuint projection_matrix_id = -1;
        GLint normal_matrix_id = -1;

        std::unique_ptr<ShaderProgram> skybox_shader;
        std::shared_ptr<Skybox> skybox;
        GLuint skybox_view_matrix_id = -1;
        GLuint skybox_projection_matrix_id = -1;

        float angle;

        // Grass system
        std::shared_ptr<GrassMesh> grassMesh;
        std::unique_ptr<ShaderProgram> grass_shader;
        GLuint grass_model_view_matrix_id = -1;
        GLuint grass_projection_matrix_id = -1;
        GLint grass_normal_matrix_id = -1;


    public:
        
        Scene(unsigned width, unsigned height);

        void update(float deltaTime);
        void render();
        void resize(unsigned width, unsigned height);
        void renderNode(const std::shared_ptr<SceneNode>& node, const glm::mat4& viewMatrix);
        std::shared_ptr<SceneNode> createNode(const std::string& name, std::shared_ptr<SceneNode> parent = nullptr);
        std::shared_ptr<SceneNode> findNode(const std::string& name, const std::shared_ptr<SceneNode>& startNode);
        void handleKeyboard(const Uint8* keyboardState);
        void updateCamera(float deltaTime);
        void resetCameraRotation();
    };
}