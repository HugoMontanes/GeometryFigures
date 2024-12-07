

//#ifndef PLANE_HEADER        //Check if PLANE_HEADER is not defined
#define PLANE_HEADER        //Define PLANE_HEADER

#include "Mesh.hpp"

#include <vector>
#include<glad/glad.h>

namespace space
{
    class Plane: public Mesh
    {
    private:

        unsigned int rows, cols;
        float width, height;

    public:

        Plane(unsigned int _rows, unsigned int _cols, float _width, float _height)
            : rows(_rows), cols(_cols), width(_width), height(_height)
        {
            initialize();
            setUpMesh();
        }

        void initialize() override;
    };

}
