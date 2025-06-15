/*
* Este código es de dominio público
* Realizado por Hugo Montañés García
*/

#include "Cube.hpp"

namespace space
{
    void Cube::initialize()
    {
        vertices.clear();
        normals.clear();
        colors.clear();
        indices.clear();

        // Calculate half dimensions for centering the cube at origin
        float halfWidth = width / 2.0f;
        float halfHeight = height / 2.0f;
        float halfDepth = depth / 2.0f;

        // Define the 8 vertices of the cube
        // Think of this as defining the corners of a box
        vertices = {
            // Front face vertices (positive Z)
            glm::vec3(-halfWidth, -halfHeight,  halfDepth),  // 0: bottom-left-front
            glm::vec3(halfWidth, -halfHeight,  halfDepth),  // 1: bottom-right-front
            glm::vec3(halfWidth,  halfHeight,  halfDepth),  // 2: top-right-front
            glm::vec3(-halfWidth,  halfHeight,  halfDepth),  // 3: top-left-front

            // Back face vertices (negative Z)
            glm::vec3(-halfWidth, -halfHeight, -halfDepth),  // 4: bottom-left-back
            glm::vec3(halfWidth, -halfHeight, -halfDepth),  // 5: bottom-right-back
            glm::vec3(halfWidth,  halfHeight, -halfDepth),  // 6: top-right-back
            glm::vec3(-halfWidth,  halfHeight, -halfDepth)   // 7: top-left-back
        };

        // Define normals for each face
        // For a cube, we need to duplicate vertices because each face needs its own normal
        // This is why we'll have 24 vertices (6 faces × 4 vertices per face) instead of just 8
        vertices.clear();
        normals.clear();
        colors.clear();

        // Helper arrays to define each face
        // Front face (positive Z)
        vertices.push_back(glm::vec3(-halfWidth, -halfHeight, halfDepth));
        vertices.push_back(glm::vec3(halfWidth, -halfHeight, halfDepth));
        vertices.push_back(glm::vec3(halfWidth, halfHeight, halfDepth));
        vertices.push_back(glm::vec3(-halfWidth, halfHeight, halfDepth));
        for (int i = 0; i < 4; i++) {
            normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
            colors.push_back(glm::vec3(0.5f, 0.7f, 1.0f)); // Light blue color
        }

        // Back face (negative Z)
        vertices.push_back(glm::vec3(halfWidth, -halfHeight, -halfDepth));
        vertices.push_back(glm::vec3(-halfWidth, -halfHeight, -halfDepth));
        vertices.push_back(glm::vec3(-halfWidth, halfHeight, -halfDepth));
        vertices.push_back(glm::vec3(halfWidth, halfHeight, -halfDepth));
        for (int i = 0; i < 4; i++) {
            normals.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
            colors.push_back(glm::vec3(0.5f, 0.7f, 1.0f));
        }

        // Top face (positive Y)
        vertices.push_back(glm::vec3(-halfWidth, halfHeight, halfDepth));
        vertices.push_back(glm::vec3(halfWidth, halfHeight, halfDepth));
        vertices.push_back(glm::vec3(halfWidth, halfHeight, -halfDepth));
        vertices.push_back(glm::vec3(-halfWidth, halfHeight, -halfDepth));
        for (int i = 0; i < 4; i++) {
            normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
            colors.push_back(glm::vec3(0.5f, 0.7f, 1.0f));
        }

        // Bottom face (negative Y)
        vertices.push_back(glm::vec3(-halfWidth, -halfHeight, -halfDepth));
        vertices.push_back(glm::vec3(halfWidth, -halfHeight, -halfDepth));
        vertices.push_back(glm::vec3(halfWidth, -halfHeight, halfDepth));
        vertices.push_back(glm::vec3(-halfWidth, -halfHeight, halfDepth));
        for (int i = 0; i < 4; i++) {
            normals.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
            colors.push_back(glm::vec3(0.5f, 0.7f, 1.0f));
        }

        // Right face (positive X)
        vertices.push_back(glm::vec3(halfWidth, -halfHeight, halfDepth));
        vertices.push_back(glm::vec3(halfWidth, -halfHeight, -halfDepth));
        vertices.push_back(glm::vec3(halfWidth, halfHeight, -halfDepth));
        vertices.push_back(glm::vec3(halfWidth, halfHeight, halfDepth));
        for (int i = 0; i < 4; i++) {
            normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
            colors.push_back(glm::vec3(0.5f, 0.7f, 1.0f));
        }

        // Left face (negative X)
        vertices.push_back(glm::vec3(-halfWidth, -halfHeight, -halfDepth));
        vertices.push_back(glm::vec3(-halfWidth, -halfHeight, halfDepth));
        vertices.push_back(glm::vec3(-halfWidth, halfHeight, halfDepth));
        vertices.push_back(glm::vec3(-halfWidth, halfHeight, -halfDepth));
        for (int i = 0; i < 4; i++) {
            normals.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
            colors.push_back(glm::vec3(0.5f, 0.7f, 1.0f));
        }

        // Define indices for the cube faces
        // Each face is made of 2 triangles, so 6 indices per face
        indices.reserve(36); // 6 faces × 2 triangles × 3 vertices

        for (int face = 0; face < 6; face++) {
            int offset = face * 4;

            // First triangle of the face
            indices.push_back(offset + 0);
            indices.push_back(offset + 1);
            indices.push_back(offset + 2);

            // Second triangle of the face
            indices.push_back(offset + 0);
            indices.push_back(offset + 2);
            indices.push_back(offset + 3);
        }
    }
}