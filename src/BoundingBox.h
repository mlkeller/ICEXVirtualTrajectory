#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <string>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;

struct BoundingBox{
public:
	BoundingBox(){
		min = vec3(1.1754E+38F);
		max = vec3(-1.1754E+38F);
	}

	BoundingBox(vec3 min, vec3 max){
		min = min;
		max = max;
	}

	vec3 min;
	vec3 max;
	Plane plane1;
	Plane plane2;
	Plane plane3;
	Plane plane4;
	Plane plane5;


	void addBoundingBox(BoundingBox b){
		if(b.min.x < min.x){
			min.x = b.min.x;
		}
		if(b.min.y < min.y){
			min.y = b.min.y;
		}
		if(b.min.z < min.z){
			min.z = b.min.z;
		}
		if(b.max.x > max.x){
			max.x = b.max.x;
		}
		if(b.max.y > max.y){
			max.y = b.max.y;
		}
		if(b.max.z > max.z){
			max.z = b.max.z;
		}
	}

	void setPlanes(){
		plane1 =  Plane(vec3(max.x,max.y,min.z),max,vec3(min.x,max.y,max.z));
		plane1.setNormal();
		//cout << "plane 1: " << plane1.normal.x<<"   " << plane1.normal.y<<"  " << plane1.normal.z<<"   "<<  plane1.normal.w<<"\n";
      plane2 = Plane(vec3(min.x,max.y,max.z),max,vec3(max.x,min.y,max.z));
      plane2.setNormal();
      //cout << "plane 2: " << plane2.normal.x<<"   " << plane2.normal.y<<"  " << plane2.normal.z<<"   "<<  plane2.normal.w<<"\n";
      plane3 = Plane(vec3(max.x,min.y,max.z),max,vec3(max.x,max.y,min.z));
      plane3.setNormal();
      //cout << "plane 3: " << plane3.normal.x<<"   " << plane3.normal.y<<"  " << plane3.normal.z<<"   "<<  plane3.normal.w<<"\n";
      plane4 = Plane(vec3(max.x,max.y,min.z),min,vec3(min.x,max.y,min.z));
      plane4.setNormal();
      //cout << "plane 4: " << plane4.normal.x<<"   " << plane4.normal.y<<"  " << plane4.normal.z<<"   "<<  plane4.normal.w<<"\n";
      plane5 = Plane(min,vec3(min.x,max.y,min.z),vec3(min.x,max.y,max.z));
      plane5.setNormal();
      //cout << "plane 5: " << plane5.normal.x<<"   " << plane5.normal.y<<"  " << plane5.normal.z<<"   "<<  plane5.normal.w<<"\n";
	}

};

#endif