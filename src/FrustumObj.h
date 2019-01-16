#ifndef FRUSTUMOBJ_H
#define FRUSTUMOBJ_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

struct FrustumObj: public Obj{
public:
	FrustumObj(){}

	FrustumObj(string dir, string Mesh, string vertShader, string fragShader, string texture, int textNum, int textType, int sType){
		RESOURCE_DIR = dir;
		mesh = Mesh; 
		vert = vertShader;
		frag = fragShader;
		textureName = texture;
		textureNum = textNum;
		textureType = textType;
	}

	GLfloat vertices[8*3];
	 //geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	//global data for ground plane
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;

void setUp(Frustum f){
    
    setVertices(f);


    float GrndNorm[] = {
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0
    };

  static GLfloat GrndTex[] = {
      0, 0, // back
      0, 22,
      22, 22,
      22, 0 };

   unsigned short idx[] = { 
   	   0,1,2,
	      2,3,0,
	      3,2,6,
	      6,3,7,
	      7,6,5,
	      7,5,4,
	      0,4,5,
	      0,5,1
	      //1,6,5
	      //1,6,2
	   };

     GLuint VertexArrayID;
	//generate the VAO
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

    g_GiboLen = 30;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &GrndNorBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);
    
	 glGenBuffers(1, &GrndTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);


	}

	void setVertices(Frustum f){
		vertices[0] = f.top.pt1.x;
		vertices[1] = f.top.pt1.y;
		vertices[2] = f.top.pt1.z;
		vertices[3] = f.top.pt2.x;
		vertices[4] = f.top.pt2.y;
		vertices[5] = f.top.pt2.z;
		vertices[6] = f.top.pt3.x;
		vertices[7] = f.top.pt3.y;
		vertices[8] = f.top.pt3.z;
		vertices[9] = f.top.pt4.x;
		vertices[10] = f.top.pt4.y;
		vertices[11] = f.top.pt4.z;
		vertices[12] = f.bottom.pt1.x;
		vertices[13] = f.bottom.pt1.y;
		vertices[14] = f.bottom.pt1.z;
		vertices[15] = f.bottom.pt2.x;
		vertices[16] = f.bottom.pt2.y;
		vertices[17] = f.bottom.pt2.z;
		vertices[18] = f.bottom.pt3.x;
		vertices[19] = f.bottom.pt3.y;
		vertices[20] = f.bottom.pt3.z;
		vertices[21] = f.bottom.pt4.x;
		vertices[22] = f.bottom.pt4.y;
		vertices[23] = f.bottom.pt4.z;
	}

	void draw(shared_ptr<MatrixStack> M, shared_ptr<MatrixStack>  P, mat4 V){
		prog->bind();
	   M->pushMatrix();
	   texture->bind(prog->getUniform("fragTexture"));
	   glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
	   glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
	   glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(V));
	   glEnableVertexAttribArray(0);
	   glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
	   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	   glEnableVertexAttribArray(1);
	   glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
	   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	   glEnableVertexAttribArray(2);
	   glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
	   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	   // draw!
	   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
	   glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

	   glDisableVertexAttribArray(0);
	   glDisableVertexAttribArray(1);
	   glDisableVertexAttribArray(2);

	   M->popMatrix();
	   prog->unbind();
	}


 

};

#endif