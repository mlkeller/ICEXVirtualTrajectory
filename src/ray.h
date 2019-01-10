#ifndef RAY_H
#define RAY_H


#include <string>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Plane.h"
using namespace std;
using namespace glm;

struct Ray {
public:
	Ray(){}

	Ray(vec3 pos, vec3 direction){
		origin = pos;
		dir = direction;
	}

	vec3 origin;
	vec3 dir;


};

#endif