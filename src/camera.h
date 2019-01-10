#ifndef CAMERA_H
#define CAMERA_H


#include <string>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Frustum.h"
#include "Plane.h"
#include "ray.h"
using namespace std;
using namespace glm;

struct Camera {
public:
	Camera(){}

	Camera(vec3 position, vec3 lookat, float speed){
		camSpeed = speed;
		eye = position;
		startPos = position;
		lookAt = lookat;
		view = lookAt - eye;
		fView = normalize(view);
		vecW = -1.0f * normalize(view);
		vecU = cross(up, vecW);
		vecV = cross(vecW, vecU);	
		rightV = normalize(cross(normalize(view),up));
		cameraView = glm::lookAt(eye,lookAt,up);
	}

	float camSpeed;
	vec3 lookAt;
	vec3 eye;
	vec3 startPos;
	vec3 fView;
	vec3 rightV;
	vec3 up = vec3(0, 1, 0);
	vec3 view;
	vec3 vecW,vecU, vecV;
	mat4 cameraView; 
	Frustum frustum; 
	float aspect;
	float FOV;
	float zNear;
	float zFar;
	vec3 NPTopRight, NPTopLeft, NPBottomRight, NPBottomLeft;
	vec3 FPTopRight, FPTopLeft, FPBottomRight, FPBottomLeft;
	Plane top = Plane();
	Plane bottom = Plane();
	Plane right = Plane();
	Plane left = Plane();
	vec3 FPnorm;
	float FPd;

	//RAYS
	vector<Ray>	rays; 
   vector<vec3> lookAts;
   float sampleNums = 9.0f;
   float weight;
   bool randomRays = false;




	void updateSpeed(float speed){
		camSpeed = speed;
	}

	void updatePosition(vec3 position){
		eye = position;
		updateCamera();
	}

	void updateLookAt(vec3 lookat){
		lookAt = lookat; 
		updateCamera();
	}

	void updateCamera(){
		view = lookAt - eye;
		vecW = -1.0f * normalize(view);
		vecU = cross(up, vecW);
		vecV = cross(vecW, vecU);	
		cameraView = glm::lookAt(eye,lookAt,up);
	}

	void updatePerspective(float aspectR, float fov, float zN, float zF){
		aspect = aspectR;
		FOV = fov;
		zNear =zN;
		zFar = zF;
	}

	void createFrustum(){
		getNearPlanePoints();
		getFarPlanePoints();
		createPlanes();
		frustum = Frustum(top, right, bottom, left);
		setPoints();
	}

	void setPoints(){
		frustum.Points.push_back(NPTopRight);
		frustum.Points.push_back(NPTopLeft);
		frustum.Points.push_back(NPBottomRight);
		frustum.Points.push_back(NPBottomLeft);
		frustum.Points.push_back(FPTopRight);
		frustum.Points.push_back(FPTopLeft);
		frustum.Points.push_back(FPBottomRight);
		frustum.Points.push_back(FPBottomLeft);
	}

	void createFarPlane(){
		vec3 AB,AC, norm;
		float d;
		AB = FPTopLeft - FPTopRight;
		AC = FPBottomRight - FPTopRight;
		norm = cross(AB,AC);
		norm = normalize(norm);
		FPnorm = norm;
		d = dot(norm,FPTopRight);
		FPd = d;

	}

	float max( float a, float b){
   	if(a >= b){
   		return a;
   	}
   	return b;
   }

   float min(float a, float b){
   	if(a <= b){
   		return a;
   	}
   	return b;

   }

   void setRayParameters(bool rand, float sampNum){
   	randomRays = rand;
   	sampleNums = sampNum;
   }

   void createLookAts(){
   	if(randomRays == 1){
   		srand ( time(NULL) );
	   	float x,y,z; // Ranodm selction of points 
	   	for(int i = 0; i < sampleNums; i ++){
	   		//X
	      	do{
		  			float numb = max(abs(FPTopRight.x),abs(FPBottomLeft.x));
		  			x = ((float)rand()/(float)(RAND_MAX)) * numb; 
		  			int n = rand() % 2;
	   			if(n == 0){
		   			x*=-1;
		   		}
		  		}while(x < FPBottomLeft.x  || x > FPTopRight.x);
		  		//Y
		  		do{
		   		float numb = max(abs(FPTopRight.y),abs(FPBottomLeft.y));
		   		y = ((float)rand()/(float)(RAND_MAX)) * numb; 
		   		int n = rand() % 2;
		  			if(n == 0){
		   			y*=-1;
		   		}
		   	}while(y < FPBottomLeft.y || y > FPTopRight.y);
		   	//Z
		  	   z = (FPd - FPnorm.x * x - FPnorm.y * y ) / FPnorm.z;
		   	//cout << "vec: " << x <<  "  " << y << "  " << z << "\n";
	   		lookAts.push_back(vec3(x,y,z));
	   	}
	   }

   }


   void createRays(){
   	for(int i = 0; i < lookAts.size(); i ++){
   		vec3 dir = lookAts[i] - startPos;
   		dir = normalize(dir);
   		Ray r = Ray(startPos,dir);
   		rays.push_back(r);
   	}
   }

   vec3 transformPoint(vec3 vec, mat4 transform, vec3 move){
   	vec4 vecTemp = vec4(vec,0);
   	vecTemp = transform*vecTemp;
   	vec = vec3(vecTemp);
   	vec += move;
   	return vec;
   }

   void transformLookAts(mat4 transform, vec3 move){
   	for(int i = 0; i < lookAts.size(); i ++){
   		lookAts[i] = transformPoint(lookAts[i],transform,move);
   	}

   }

	void getNearPlanePoints(){
		float hNear = 2 * tan(radians(FOV)/2.0)*zNear; // height of near plane 
		float wNear = hNear * aspect; //width of near plane 
		vec3 cNear = startPos + fView*zNear; //center of near plane 

		vec3 straight = vec3(1,0,0);
		float theta = dot(fView,straight)/(length(straight)*length(fView));

		NPTopRight = vec3( wNear / 2, hNear / 2, -zNear ) ;
		NPBottomLeft = vec3( -wNear / 2, -hNear / 2, -zNear ) ;
      NPTopLeft = vec3( -wNear / 2, hNear / 2, -zNear ) ;
      NPBottomRight = vec3( wNear / 2, -hNear / 2, -zNear ) ;

      mat4 IcameraView = inverse(cameraView = glm::lookAt(startPos,lookAt,up));

          
      NPTopRight = transformPoint(NPTopRight,IcameraView,cNear);
      NPTopLeft = transformPoint(NPTopLeft,IcameraView,cNear);
      NPBottomRight = transformPoint(NPBottomRight,IcameraView,cNear);
      NPBottomLeft = transformPoint(NPBottomLeft,IcameraView,cNear);

   }
	
	void getFarPlanePoints(){  // this not only gets the far plane but also makes the rays 

		float hFar = 2 * tan(radians(FOV/2.0f))*zFar; //height of far plane 
		float wFar = hFar * aspect; // width of far plane 
		vec3 cNear = startPos + fView*zNear; 
		mat4 IcameraView = inverse(cameraView = glm::lookAt(startPos,lookAt,up));

		FPTopRight = vec3( wFar / 2, hFar / 2, -zFar );
		FPBottomLeft = vec3( -wFar / 2, -hFar / 2, -zFar );
      FPTopLeft = vec3( -wFar / 2, hFar / 2, -zFar );
      FPBottomRight = vec3( wFar / 2, -hFar / 2, -zFar );

      createFarPlane();
      createLookAts();
      transformLookAts(IcameraView,cNear);
      createRays();
       
      FPTopRight = transformPoint(FPTopRight,IcameraView,cNear);
      FPTopLeft = transformPoint(FPTopLeft,IcameraView,cNear);
      FPBottomRight = transformPoint(FPBottomRight,IcameraView,cNear);
      FPBottomLeft = transformPoint(FPBottomLeft,IcameraView,cNear);
	}

	void createPlanes(){
		//cout << "hi: " << NPTopLeft.x << "\n";
		top.updatePlane(NPTopLeft, FPTopLeft, FPTopRight, NPTopRight);
		right.updatePlane(NPTopRight, FPTopRight, FPBottomRight, NPBottomRight);
		bottom.updatePlane(NPBottomLeft, FPBottomLeft, FPBottomRight, NPBottomRight);
		left.updatePlane(NPBottomLeft, FPBottomLeft, FPTopLeft, NPTopLeft);
	}

	

};

#endif