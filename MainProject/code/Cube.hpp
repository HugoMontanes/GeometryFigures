/*
* Este c�digo es de dominio p�blico
* Realizado por Hugo Monta��s Garc�a
*/

#pragma once

#include "Mesh.hpp"

namespace space
{
    class Cube : public Mesh
    {
    private:
        float width, height, depth;

    public:
        // Constructor with dimensions
        Cube(float _width = 2.0f, float _height = 2.0f, float _depth = 2.0f)
            : width(_width), height(_height), depth(_depth)
        {
            initialize();
            setUpMesh();
        }

        void initialize() override;
    };
}