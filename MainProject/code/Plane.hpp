/*
* Este código es de dominio público
* Realizado por Hugo Montañés García
*/

#pragma once

#include "Mesh.hpp"

#include <vector>

namespace space
{
    class Plane: public Mesh
    {
    private:

        unsigned int rows, cols;
        float width, height;

    public:

        Plane(unsigned int _rows = 1, unsigned int _cols = 1, float _width = 1.0f, float _height = 1.f)
            : rows(_rows), cols(_cols), width(_width), height(_height)
        {
            initialize();
            setUpMesh();
        }

        void initialize() override;
    };

}
