
#pragma once

#include "Mesh.hpp"

#include <cmath>

namespace space
{
	class Cone : public Mesh
	{
	private:
		unsigned int slices;
		float radius, height;

	public:

		Cone(unsigned int _slices = 10, float _radius = 1.0f, float _height = 2.f)
			:slices(_slices), radius(_radius), height(_height)
		{
			initialize();
			setUpMesh();
		}

		void initialize() override;
	};
}