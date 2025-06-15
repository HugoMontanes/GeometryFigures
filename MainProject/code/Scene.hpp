/*
* Este código es de dominio público
* Realizado por Hugo Montañés García
*/

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
#include "Cube.hpp"

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

        //Transparent objects
        std::shared_ptr<SceneNode> transparentCubeNode;
        std::unique_ptr<ShaderProgram> transparent_shader;
        GLuint transparent_model_view_matrix_id = -1;
        GLuint transparent_projection_matrix_id = -1;
        GLint transparent_normal_matrix_id = -1;
        GLint transparency_uniform_id = -1;

        // Rotation control for the transparent cube
        float cubeRotationSpeed = 1.0f;  // Radians per second
        float cubeRotationAngle = 0.0f;  // Current rotation angle



    public:
        
        Scene(unsigned width, unsigned height);

        void update(float deltaTime);
        void render();
        void renderOpaqueNodes(const std::shared_ptr<SceneNode>& node, const glm::mat4& viewMatrix);
        void resize(unsigned width, unsigned height);
        void renderNode(const std::shared_ptr<SceneNode>& node, const glm::mat4& viewMatrix);
        std::shared_ptr<SceneNode> createNode(const std::string& name, std::shared_ptr<SceneNode> parent = nullptr);
        std::shared_ptr<SceneNode> findNode(const std::string& name, const std::shared_ptr<SceneNode>& startNode);
        void handleKeyboard(const Uint8* keyboardState);
        void toggleCubeRotation();
        void updateCamera(float deltaTime);
        void resetCameraRotation();

        void updateTransparencyAnimation(float deltaTime);

        void setTransparency(float alpha) {
            if (transparent_shader && transparency_uniform_id != -1) {
                transparent_shader->use();
                glUniform1f(transparency_uniform_id, alpha);
            }
        }
    };
}