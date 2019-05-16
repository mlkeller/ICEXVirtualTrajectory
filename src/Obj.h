#ifndef OBJ_H
#define OBJ_H


#include <string>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;

struct Obj{
public:
	Obj(){}

	Obj(string dir, string Mesh, string vertShader, string fragShader, string texture, int textNum, int textType, int sType){
		RESOURCE_DIR = dir;
		mesh = Mesh; 
		vert = vertShader;
		frag = fragShader;
		textureName = texture;
		textureNum = textNum;
		textureType = textType;
		shapeType = sType;
	}

	string RESOURCE_DIR; 
	string mesh;
	BoundingBox bb = BoundingBox();
	string vert;
	string frag;
	string textureName;
	int textureNum;
	int textureType;
	shared_ptr<Program> prog;
	shared_ptr<Shape> shape;
	shared_ptr<Texture> texture;
	int shapeType;
	std::vector<shared_ptr<Shape>> AllShapes;
	vec3 trans, scale, rot;
	float angle;
	vec3 BBtrans, BBscale, BBrot;
	
	void addTransforms(vec3 translate, vec3 Scale, float theta, vec3 rotate){
		trans = translate;
		scale = Scale;
		angle = theta;
		rot = rotate;
	
	}
	
	void initializeShape(){
		if(shapeType == 0){
			shape = make_shared<Shape>();
	    	shape->loadMesh(RESOURCE_DIR + mesh);
	      shape->resize();
	      shape->init();
	   }
	}

    void initializeProg(){
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(RESOURCE_DIR + vert, RESOURCE_DIR + frag);
		prog->init();
	}

	void initializeTexture(){
	   texture = make_shared<Texture>();
	   texture->setFilename(RESOURCE_DIR + textureName);
	   texture->init();
	   texture->setUnit(textureNum);
	   if(textureType ==0){
	      texture->setWrapModes(GL_REPEAT,GL_REPEAT);
	   }else if(textureType ==1){
	      texture->setWrapModes(GL_MIRRORED_REPEAT,GL_MIRRORED_REPEAT);
	   }else if(textureType ==2){
	      texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	   }else if(textureType ==3){
          texture->setWrapModes(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
      }/*
      else {
          glBindTexture(GL_TEXTURE_CUBE_MAP, textureType);
          glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

      }*/
	}

	void addUniforms(){
   	prog->addUniform("P");
	 	prog->addUniform("M");
		prog->addUniform("V");
		prog->addAttribute("vertPos");
      prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
      prog->addUniform("fragTexture");
	}

	void draw(shared_ptr<MatrixStack> M, shared_ptr<MatrixStack>  P, mat4 V){

		prog->bind();
		texture->bind(prog->getUniform("fragTexture"));
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

	float avg(float num1, float num2){
		return (num1+num2)/2.0;
	}

   void createBB(){
		double *locArr = new double[1000];
		vector<tinyobj::shape_t> TOshapes;
		vector<tinyobj::material_t> objMaterials;
		string errStr;
		vec3 Gmin = vec3(1.1754E+38F);
		vec3 Gmax = vec3(-1.1754E+38F);
		vec3 g_trans;
		float g_scale = 0;
		//load in the mesh and make the shapes
		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (RESOURCE_DIR + mesh).c_str());
		if (!rc) {
			cerr << errStr << endl;
		}
		else {
			//some data to keep track of where our mesh is in space
			int j = 0;
			for (int i = 0; i < TOshapes.size(); i++) {
				// TODO -- Initialize each mesh
				//1. make a shared pointer
				shared_ptr<Shape> s = std::make_shared<Shape>();
				// 2. createShape for each tiny obj shape
				s->createShape(TOshapes[i]);
				// 3. measure each shape to find out its AABB
				s->measure();
				// 4. call init on each shape to create the GPU data
				s->init();
				//perform some record keeping to keep track of global min and max
				if (s->min.x < Gmin.x) {
					Gmin.x = s->min.x;
				}
				if (s->min.y < Gmin.y) {
					Gmin.y = s->min.y;
				}
				if (s->min.z < Gmin.z) {
					Gmin.z = s->min.z;
				}
				if (s->max.x > Gmax.x) {
					Gmax.x = s->max.x;
				}
				if (s->max.y > Gmax.y) {
					Gmax.y = s->max.y;
				}
				if (s->max.z > Gmax.z) {
					Gmax.z = s->max.z;
				}
				//Add the shape to AllShapes
				AllShapes.push_back(s);
				locArr[j] = s->min.x;
			   locArr[j + 1] = s->max.x;
				locArr[j + 2] = s->min.y;
				locArr[j+ 3] = s->max.y;
				locArr[j + 4] = s->min.z;
				locArr[j + 5] = s->max.z;
				j += 6;
			}
			//fix these based on the results of calling measure on each peice	
			float zDiff = Gmax.z - Gmin.z;
			float xDiff = Gmax.x - Gmin.x;
			float yDiff = Gmax.y - Gmin.y;
			float maxDiff = zDiff;
			if (xDiff > maxDiff) {
				maxDiff = xDiff;
			}
			if (yDiff > maxDiff) {
				maxDiff = yDiff;
			}
			//printf("maxDiff : %f\n", maxDiff);
			g_trans = vec3((Gmax.x + Gmin.x) / 2, (Gmax.y + Gmin.y) / 2, (Gmax.z + Gmin.z) / 2);
			g_scale = 1 / maxDiff;
		}
		bb.min = Gmin;
		bb.max = Gmax;
		bb.min *= scale; 
		bb.max *= scale;
		bb.min += trans;
		bb.max += trans;

		BBscale = vec3((bb.max.x - bb.min.x)/2.0,(bb.max.y - bb.min.y)/2.0,(bb.max.z - bb.min.z)/2.0);	
   	BBtrans = vec3(avg(bb.min.x,bb.max.x),avg(bb.min.y,bb.max.y),avg(bb.min.z,bb.max.z));

	}

	void printMatrices(shared_ptr<MatrixStack> M, shared_ptr<MatrixStack>  P, mat4 V){
		cout << "print P\n";
		P->print(P->topMatrix(),0);
		cout << "print M\n";
		M->print(M->topMatrix(),0);

		cout << "V " << "\n" << V[0][0] << " " << V[0][1] << " " << V[0][2] << " " << V[0][3] << "\n";
		cout << V[1][0] << " " << V[1][1] << " " << V[1][2] << " " << V[1][3] << "\n";
   	cout << V[2][0] << " " << V[2][1] << " " << V[2][2] << " " << V[2][3] << "\n";
   	cout << V[3][0] << " " << V[3][1] << " " << V[3][2] << " " << V[3][3] << "\n" << "\n";

	}

};

#endif