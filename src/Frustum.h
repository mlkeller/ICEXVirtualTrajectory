#include <string>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Plane.h"
using namespace std;
using namespace glm;

struct Frustum {
public:
	Frustum(){}

	Frustum(Plane t, Plane r, Plane b, Plane l){
		top = t;
		right = r;
		bottom = b;
		left = l;

	}

	Plane top, right, bottom, left;
	std::vector<vec3> Points;
	std::vector<vec4> Planes;


};