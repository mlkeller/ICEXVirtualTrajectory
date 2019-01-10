#ifndef PLANE_H
#define PLANE_H

#include <string>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;

struct Plane {
public:
	vec3 pt1, pt2, pt3, pt4;
	vec4 normal;
	

	Plane(){}

	Plane(vec3 p1, vec3 p2, vec3 p3, vec3 p4){
		pt1 = p1;
		pt2 = p2;
		pt3 = p3;
		pt4 = p4;
	}

	Plane(vec3 p1, vec3 p2, vec3 p3){
		pt1 = p1;
		pt2 = p2;
		pt3 = p3;
	}


	void updatePlane(vec3 p1, vec3 p2, vec3 p3, vec3 p4){
		pt1 = p1;
		pt2 = p2;
		pt3 = p3;
		pt4 = p4;
	}

	void setNormal(){
		vec3 a = pt1-pt2;
		vec3 b = pt3-pt2;
		vec3 n = cross(a,b);
		n = normalize(n);
		float d = abs(-n.x*pt2.x-n.y*pt2.y-n.z*pt2.z);
		normal = vec4(n.x,n.y,n.z,d);
	}

};

#endif 