#ifndef ANCHOR_H
#define ANCHOR_H


#include <string>
#include <fstream>
#include <iostream>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "FrustumObj.h"
#include "ray.h"
#include "Utilities.h"
using namespace std;
using namespace glm;

#define ALPHA .33
#define BETA .33
#define GAMMA  .33

struct Anchor : public Obj {
public:
    Anchor() {
    }

    Anchor(string dir, string Mesh, string vertShader, string fragShader, string texture, int textNum, int textType, int sType) {
        RESOURCE_DIR = dir;
        mesh = Mesh;
        vert = vertShader;
        frag = fragShader;
        textureName = texture;
        textureNum = textNum;
        textureType = textType;
        shapeType = sType;
        anchorType = -1;
        parentIndex = -1;
        pathLength = 1;
    }

    float anchorType;
    Camera camera;
    vec3 pos = vec3(-1.0f);
    vec3 view;
    FrustumObj fObj;
    float sampleNums;
    float weight = 0;
    float depth;
    float camPhi;
    float camTheta;
    int parentIndex;
    float robotTheta;
    int pathLength;
    int ndex;
    int radius = 80;
    int root = 0;
    unsigned int hitting = 0;
    int newSideSeen = 0;
    vec3 velocity = vec3(1.0);
    vec3 auvSpeed = vec3(3.0);


    void createLookAt(vec3 origin, vec3 *lookAt) {
        float radius = 6;
        float theta;
        vec3 pitch, yaw;
        //vec3 temp = vec3(origin.x+.001,origin.y-100, origin.z+.001);
        vec3 temp = vec3(0, 0, 0);
        vec3 look = vec3(0, -1, 0);

        camPhi = rand() / float(RAND_MAX);
        camPhi = (1.0f - camPhi) * -20 + camPhi * 20;
        //cout << "phi: " << camPhi << "\n";
        float delta = radians(camPhi);
        pitch.x = 0;
        pitch.z = look.z*cos(delta) - look.y*sin(delta);
        pitch.y = look.y*cos(delta) - look.z*sin(delta);
        //pitch = normalize(pitch);
        //cout << "pitch: " << pitch.x  << " " << pitch.y << " " << pitch.z << "\n";
        yaw = getPerpendicularVector(pitch);
        //yaw = normalize(yaw);    
        camTheta = atan(yaw.x / yaw.z);
        //cout << "yaw: " << yaw.x  << " " << yaw.y << " " << yaw.z << "\n";
        //cout << "theta: " << camTheta << "\n";


        vec3 dir = pitch + yaw;

        //cout << "dir: " << dir.x  << " " << dir.y << " " << dir.z << "\n";
        temp = dir;
        lookAt->x = temp.x;
        lookAt->y = temp.y;
        lookAt->z = temp.z;

        /*
        lookAt->x =rand()%1000;n
        lookAt->y =rand()%1000;
        lookAt->z =rand()%1000;
        */
        /*
        lookAt->x = 0;
        lookAt->y = 0;
        lookAt->z = 0;
        */
    }

    vec3 getPerpendicularVector(vec3 v) {
        float y;
        y = (-v.x - v.z) / v.y;
        return vec3(1, y, 1);
    }


    void setPositionAndLookAt(Anchor *prev, vec3 *lookAt, vec3 *pos, int iteration, vector<Anchor> roadMap) {
        *lookAt = prev->camera.lookAt;
        *pos = prev->pos;

        vec3 pitch, yaw;
        vec3 look = vec3(0, -1, 0); //camera is looking straight down 
        //srand(time(0));
        float delta = rand() / float(RAND_MAX);
        delta = (1.0f - delta) * -20 + delta * 20;
        cout << "delta: " << delta << "\n";
        //cout << "prev Phi: " << prev->camPhi << " \nprev Theta: " << prev->camTheta << "\n";; 
        //if(iteration%2 == 0){ //even iteration change yaw (effects x and z) by 20 degrees and set position
        camPhi = prev->camPhi + delta;
        if (camPhi > 20) {
            camPhi = 20;
        }
        else if (camPhi < -20) {
            camPhi = -20;
        }
        //cout << "phi: " << camPhi << "\n";

        delta = radians(camPhi);

        pitch.x = 0;
        pitch.y = look.y*cos(delta) - look.z*sin(delta);
        pitch.z = look.z*cos(delta) - look.y*sin(delta);

        yaw = getPerpendicularVector(pitch);
		/*
        camTheta = prev->camTheta + delta;
        if (camTheta > 20) {
            camTheta = 20; 
        }
        else if (camTheta < -20) {
            camTheta = -20;
        }

        delta = radians(camTheta);

        yaw.x = look.x*cos(delta) - look.z*sin(delta);
        yaw.y = 0;
        yaw.z = look.z*cos(delta) - look.x*sin(delta);*/
   
        camTheta = atan(yaw.x / yaw.z);
        camTheta = degrees(camTheta);

		if (this->hitting > roadMap[this->parentIndex].hitting) { //If the child sees a new side
			this->velocity = normalize(cross(pitch, yaw));
			this->newSideSeen = 1;
		}
		else if (0) { //If side that was seen is not seen anymore

		}
		else if (this->newSideSeen == 1) { //If new side has been seen
			this->velocity = roadMap[this->parentIndex].velocity;
		}
		else if (this->newSideSeen == 0) {
			this->velocity = normalize(cross(pitch, yaw));
			/*this->velocity.x = cos(pitch.x) * cos(yaw.x);
			this->velocity.y = sin(yaw.y);
			this->velocity.z = sin(pitch.z) * cos(yaw.z);*/
		}
        this->velocity = normalize(cross(pitch, yaw));
        // cout << "yaw: " << yaw.x << " " << yaw.y << " " << yaw.z << "\n";
        // cout << "pitch: " << pitch.x << " " << pitch.y << " " << pitch.z << "\n";
        *pos = prev->pos + this->velocity;
        //cout << "pos: " << pos->x << " " << pos->y << " " << pos->z << "\n";



        delta = rand() / float(RAND_MAX);
        delta = (1.0f - delta) * -20 + delta * 20;
        camPhi = prev->camPhi + delta;
        if (camPhi > 20) {
            camPhi = 20;
        }
        else if (camPhi < -20) {
            camPhi = -20;
        }
        delta = radians(camPhi);

        pitch.x = 0;
        pitch.z = look.z*cos(delta) - look.y*sin(delta);
        pitch.y = look.y*cos(delta) - look.z*sin(delta);

        yaw = getPerpendicularVector(pitch);
        *lookAt = prev->camera.lookAt + this->velocity;
    }

	void calculatePosition(Anchor *prev, vec3 *pos, vec3 *velocity, vector<Anchor> roadMap, BoundingBox bb)
	{
       bool validPos = false;
       int count = 0;
       while (!validPos)
       {
           *pos = prev->pos;
           vec3 prevVelocity = normalize(roadMap[this->parentIndex].velocity);
           /*int isInf = isnan(prevVelocity.x);
           if (isInf == 1)
           {
               prevVelocity.x = rand() / float(RAND_MAX);
               prevVelocity.y = rand() / float(RAND_MAX);
               prevVelocity.z = rand() / float(RAND_MAX);
           }*/
           //cout << "velocity: " << prevVelocity.x << " " << prevVelocity.y << " " << prevVelocity.z << "\n";

           float phi = asin(prevVelocity.y);
           float theta = atan2(prevVelocity.x, prevVelocity.z) - M_PI;

           float pitchDelta = M_PI * (rand() / float(RAND_MAX));
           float yawDelta = M_PI * (rand() / float(RAND_MAX));

           phi += pitchDelta;
           theta += yawDelta;

           vec3 newVelocity;
           newVelocity.x = cos(phi) * sin(M_PI + theta) * 5.0f;
           newVelocity.y = sin(phi) * 5.0f;
           newVelocity.z = cos(phi) * cos(M_PI - theta) * 5.0f;

           newVelocity *= auvSpeed;

          // cout << "velocity: " << newVelocity.x << " " << newVelocity.y << " " << newVelocity.z << "\n";
           this->velocity = newVelocity;
           *velocity = this->velocity;
           *pos = prev->pos + this->velocity;

           if ((pos->x < bb.min.x || pos->x > bb.max.x) || 
               (pos->y < bb.min.y || pos->y > bb.max.y) || 
               (pos->z < bb.min.z || pos->z > bb.max.z))
           {
               validPos = true;
           }
           count++;

           if (count >= 5)
           {
               *pos = -1.0f * prev->pos;
               *velocity = -1.0f * prev->velocity;
           }
       }
	}

	void calculateLookAt(vec3 *lookAt, vec3 velocity)
	{
		vec3 up = vec3(0.0f, 1.0f, 0.0f);
		vec3 right = cross(velocity, up);
		*lookAt = cross(velocity, right);
	}

	void createAnchor(int iteration,Anchor *prev, int numNodes, int pathlength, float aspect, float zNear,
                     BoundingBox bb, int hits[], vector<Anchor> roadMap, vec3 realMin, vec3 realMax, bool isValidate){
      vec3 lookAt;
      vec3 position;
	   vec3 velocity;

      if (!isValidate)
      {
          calculatePosition(prev, &position, &velocity, roadMap, bb);
          calculateLookAt(&lookAt, velocity);

          // setPositionAndLookAt(prev, &lookAt, &position, iteration, roadMap); 
          pos = position;

          // make sure position is valid
          if (pos.x < realMin.x)
          {
              pos.x = realMin.x;
          }
          if (pos.y < realMin.y)
          {
              pos.y = realMin.y;
          }
          if (pos.z < realMin.z)
          {
              pos.z = realMin.z;
          }
          if (pos.x > realMax.x)
          {
              pos.x = realMax.x;
          }
          if (pos.y > realMax.y)
          {
              pos.y = realMax.y;
          }
          if (pos.z > realMax.z)
          {
              pos.z = realMax.z;
          }

         /* cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << "\n";
          cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << "\n"; */
      }
      else
      {
          position = this->pos;
          lookAt = this->camera.lookAt;
      }

      pathLength = pathlength;
      Camera c = Camera(position, lookAt, 1);
      c.updatePerspective(aspect, 45, zNear, 90);
      c.setRayParameters(1,200);
      c.createFrustum();
      setCamera(c,c.startPos.y);
      if (isValidate && this->weight == -1)
      {
          weight = 0.0f;
      }
      else
      {
          getWeight(bb, hits, roadMap);
      }
   }

   void setCamera(Camera c,float Depth){
      camera = c;
      depth = Depth;
      robotTheta = randRangef(0.0f, M_PI);
   }

   bool rayBBIntersection(Ray r, BoundingBox b, int hits[], vector<Anchor> roadMap){
      bool hit = false;
      float tmin = (b.min.x - r.origin.x) / r.dir.x; 
      float tmax = (b.max.x - r.origin.x) / r.dir.x; 
 
       if (tmin > tmax) swap(tmin, tmax); 
    
       float tymin = (b.min.y - r.origin.y) / r.dir.y; 
       float tymax = (b.max.y - r.origin.y) / r.dir.y; 
    
       if (tymin > tymax) swap(tymin, tymax); 
    
       if ((tmin > tymax) || (tymin > tmax)) 
           return hit; 
    
       if (tymin > tmin) 
           tmin = tymin; 
    
       if (tymax < tmax) 
           tmax = tymax; 
    
       float tzmin = (b.min.z - r.origin.z) / r.dir.z; 
       float tzmax = (b.max.z - r.origin.z) / r.dir.z; 
    
       if (tzmin > tzmax) swap(tzmin, tzmax); 
    
       if ((tmin > tzmax) || (tzmin > tmax)) 
           return hit; 
    
       if (tzmin > tmin) 
           tmin = tzmin; 
    
       if (tzmax < tmax) 
           tmax = tzmax; 


      hit = true;

      if(hit == true){
         checkBBPlanes(r,b,hits, roadMap);
      }

      return hit;
   }

   void checkBBPlanes(Ray r, BoundingBox b,int hits[], vector<Anchor> roadMap)
   {
      vector<float> tValues;

      //check plane1
      float t1 = b.plane1.normal.w - dot(r.origin,vec3(b.plane1.normal.x,b.plane1.normal.y,b.plane1.normal.z));
      //cout << "t: " << b.plane1.normal.w<<"\n";
      t1 = t1 / dot(r.dir,vec3(b.plane1.normal.x,b.plane1.normal.y,b.plane1.normal.z));
      //check plane2
      float t2 = b.plane2.normal.w - dot(r.origin,vec3(b.plane2.normal.x,b.plane2.normal.y,b.plane2.normal.z));
      t2 = t2 / dot(r.dir,vec3(b.plane2.normal.x,b.plane2.normal.y,b.plane2.normal.z));
      //cout << "t2: " << t2 <<"\n";
      //check plane3
      float t3 = b.plane3.normal.w - dot(r.origin,vec3(b.plane3.normal.x,b.plane3.normal.y,b.plane3.normal.z));
      t3 = t3 / dot(r.dir,vec3(b.plane3.normal.x,b.plane3.normal.y,b.plane3.normal.z));
      //cout << "t3: " << t3 <<"\n";
      //check plane4
      float t4 = b.plane4.normal.w - dot(r.origin,vec3(b.plane4.normal.x,b.plane4.normal.y,b.plane4.normal.z));
      t4 = t4 / dot(r.dir,vec3(b.plane4.normal.x,b.plane4.normal.y,b.plane4.normal.z));
      //cout << "t4: " << t4 <<"\n";
      //check plane5
      float t5 = b.plane5.normal.w - dot(r.origin,vec3(b.plane5.normal.x,b.plane5.normal.y,b.plane5.normal.z));
      t5 = t5 / dot(r.dir,vec3(b.plane5.normal.x,b.plane5.normal.y,b.plane5.normal.z));
      //cout << "t5: " << t5 <<"\n";


      if(t1 > 0){
         tValues.push_back(t1);
      }
      if(t2 > 0){
         tValues.push_back(t2);
      }
      if(t3 > 0){
         tValues.push_back(t3);
      }
      if(t4 > 0){
         tValues.push_back(t4);
      }
      if(t5 > 0){
         tValues.push_back(t5);
      }
      
      sort(tValues.begin(), tValues.end());
      //tValues.sort();
      
      //cout << "tvalue: " << tValues[0] << "/n" << endl;

      if (tValues.size() > 0) {
         float t = tValues.at(0);


         if(this->parentIndex == -1) {
            if(t == t1){
               //hit plane1;
               this->hitting = 16;
            }else if(t == t2){
               //hit plane2;
               this->hitting = 8;
            }else if(t == t3){
               //hit plane3;
               this->hitting = 4;
            }else if(t == t4){
               //hit plane4;
               this->hitting = 2;
            }else if(t == t5){
               //hit plane5;
               this->hitting = 1;
            }
         }
         else if (this->parentIndex >= 0) {
            if(t == t1){
               //hit plane1;
               //cout << "RoadMap size: " << roadMap.size() << "\n";
               this->hitting = roadMap[this->parentIndex].hitting | 16;
            }else if(t == t2){
               //hit plane2;
               //cout << "ParentIndex: " << this->parentIndex << "\n";
               this->hitting = roadMap[this->parentIndex].hitting | 8;
            }else if(t == t3){
               //hit plane3;
               //cout << "ParentIndex: " << this->parentIndex << "\n";
               this->hitting = roadMap[this->parentIndex].hitting | 4;
            }else if(t == t4){
               //hit plane4;
               //cout << "ParentIndex: " << this->parentIndex << "\n";
               this->hitting = roadMap[this->parentIndex].hitting | 2;
            }else if(t == t5){
               //hit plane5;
               //cout << "ParentIndex: " << this->parentIndex << "\n";
               this->hitting = roadMap[this->parentIndex].hitting | 1;
            }
         }
      }
   }



   void getWeight(BoundingBox b, int hits[], vector<Anchor> roadMap){
      int intersections = 0;
      sampleNums = camera.rays.size();
      for(int i = 0; i < camera.rays.size(); i++){   
         intersections += rayBBIntersection(camera.rays[i],b,hits, roadMap);
      }

      //If seeing a side and path length increasing 
      weight = intersections/sampleNums * 0.5f;
      if (parentIndex != -1) {
         if (this->hitting > roadMap[this->parentIndex].hitting) {
            weight += (intersections/sampleNums) * 0.25f;
         }
         // If weight of a child is greater than weight of parent
         if (weight > roadMap[this->parentIndex].weight) {
            weight += 0.5f * (intersections/sampleNums);
         }
      }


      //cout << "intersections: " << intersections << "\n"; 
      
   }

   void addPosition(vec3 loc){
      pos = loc;
   }


   void addObjInfo(string dir, string Mesh, string vertShader, string fragShader, string texture, int textNum, int textType, int sType){
      RESOURCE_DIR = dir;
      mesh = Mesh; 
      vert = vertShader;
      frag = fragShader;
      textureName = texture;
      textureNum = textNum;
      textureType = textType;
      shapeType = sType;
   }

   void addUniforms(){
      prog->addUniform("P");
      prog->addUniform("M");
      prog->addUniform("V");
      prog->addAttribute("vertPos");
      prog->addAttribute("vertNor");
      prog->addAttribute("vertTex");
      prog->addUniform("anchorType");
      prog->addUniform("fragTexture");
   }

   void updateAnchorType(int type){
      anchorType = type;
   }

   void draw(shared_ptr<MatrixStack> M, shared_ptr<MatrixStack>  P, mat4 V){

      prog->bind();
      texture->bind(prog->getUniform("fragTexture"));
      glUniform1f(prog->getUniform("anchorType"),anchorType);
      glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
      M->pushMatrix();
      M->translate(trans);
      M->scale(scale);
      if(angle != 0){
         M->rotate(angle, rot);
      }
      glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(V));
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
      if(shapeType == 0){
         shape->draw(prog);
      }else{
         for (int i = 0; i < AllShapes.size() ; i++) {
            AllShapes[i]->draw(prog);
         }
      }
      
      M->popMatrix();
      prog->unbind();
   }

};

#endif