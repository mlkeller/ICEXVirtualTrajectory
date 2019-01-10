/* Base code for texture mapping lab */
/* includes three images and three meshes - Z. Wood 2016 */
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include <queue>

#include "GLSL.h"
#include "glm/ext.hpp"

#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "CameraParser.h"
#include "camera.h"
#include "BoundingBox.h"
#include "Obj.h"
#include "Anchor.h"
#include "FrustumObj.h"
#include "Plane.h"
#include "funcs.h"
//#include "PRMNode.h"
//#include "BosPRMAlg.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


using namespace std;
using namespace glm;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from

int g_GiboLen;
float g_width, g_height;
float cTheta = 0;
float cHeight = 0;
float PI = 3.14;

//camera
double posX, posY;
double curX = 600;
double curY = 400;
int firstMouse = 0;
float lookX = 0;
float lookY = 0;
float lookZ = -1;
float alpha = 0;
float beta = PI / 2;
int cameraType = 0; //0 for wasd and 1 for rotating 
Camera rotCam = Camera();
Camera wasdCam = Camera(vec3(-42,56,-42),vec3(0, 0, 0),1); //(20,20,20)
Camera testCam = Camera(vec3(-50,50,50),vec3(0,0,0),1);
Camera currentCam;

Anchor testAnchor = Anchor ();

int camIndex =0;
long maxCamIndex = 0;
int imageNum = 0;

//makes sampling box
float sampleW, sampleH, sampleD, sampleDelta;
vector<Anchor> grid; 
vector<Anchor> anchorPts; 
vec3 Gmin = vec3(1.1754E+38F);
vec3 Gmax = vec3(-1.1754E+38F);
int sceneSize = 6;
vec3 sceneSizeMax = vec3(sceneSize,sceneSize,sceneSize);
vec3 sceneSizeMin = vec3(sceneSize,1,sceneSize);
vec3 lawnMowerSizeMax = vec3(3,3,3);
vec3 lawnMowerSizeMin = vec3(3,1,3);
float distFromWreck = 4;
vec3 distanceFromWreck = vec3(distFromWreck,distFromWreck,distFromWreck);

//Hashmap

//ROAD MAP
vector<Anchor> highestWeightNodes;
vector<Anchor> Nodes;
vector<Anchor> roadMap;
vector<Anchor> highWeightNodes;
float nodeWeightMean=0;
float highLevelCutOff=0;
int iteration=0;
int pitchIteration=0;
int highestNodeIndex = 0; 
int PATHNUM = 0;
int lawnmowerIndex = 0;
vector<Anchor> path;
Anchor rootNode = Anchor();
int hits[5];
float highWeightAvg; 


//objects
vector<Obj> sceneObjects; 
Obj S;
Obj testS;
Obj wreck; 
Obj wreck2;
Obj ground;
Obj sky;
Obj boundingBox;

FrustumObj frustumObj, frustumObjTest; 
Anchor anchor;
BoundingBox globalBB = BoundingBox();

vector<Camera> lawnmower; 

//rendering options
bool drawBoundingBoxes = false;
bool drawAnchorPoints = false;
bool drawGrid = false;
bool drawNodes = false;
bool drawFrustums = false;
bool drawMainBB = false;
bool printLoc = false;
bool debug = true;
bool showPath = false;
bool useLawnMowerPath = false;
bool useCircularPath = false;
bool useNodePath = false;



//FBO for mirror
GLuint frameBuf[1];
GLuint texBuf[2];
GLuint depthBuf;
//geometry for texture render
GLuint quad_VertexArrayID;
GLuint quad_vertexbuffer;

//global data for ground plane
GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;

int width, height;
float aspect;
float FOV = 45.0f;
float zNear = 0.01f;
float zFar = 500.0f;

int sampleNums = 1500;
int stratifiedNum = 10;
int drawF = 0;

vec3 globalBBtrans, globalBBscale;

float avg(float num1, float num2){
		return (num1+num2)/2.0;
}

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	//move camera 
    else if (key == GLFW_KEY_S) {
		wasdCam.eye += wasdCam.camSpeed*wasdCam.vecW;
		wasdCam.lookAt += wasdCam.camSpeed*wasdCam.vecW;
	}
	else if (key == GLFW_KEY_A) {
		wasdCam.eye -= wasdCam.camSpeed*wasdCam.vecU;
		wasdCam.lookAt -= wasdCam.camSpeed*wasdCam.vecU;
	}
	else if (key == GLFW_KEY_W) {
		wasdCam.eye -= wasdCam.camSpeed*wasdCam.vecW;
		wasdCam.lookAt -= wasdCam.camSpeed*wasdCam.vecW;
	}
	else if (key == GLFW_KEY_D) {
		wasdCam.eye += wasdCam.camSpeed*wasdCam.vecU;
		wasdCam.lookAt += wasdCam.camSpeed*wasdCam.vecU;
	}else if (key == GLFW_KEY_B){
		drawBoundingBoxes = true;
	}else if (key == GLFW_KEY_N){
		drawBoundingBoxes = false;
	}else if (key == GLFW_KEY_K){
		drawGrid = true;
	}else if (key == GLFW_KEY_L){
		drawGrid = false;
	}else if (key == GLFW_KEY_O){
		drawNodes = true;
	}else if (key == GLFW_KEY_P){
		drawNodes = false;
	}else if (key == GLFW_KEY_U){
		drawAnchorPoints = true;
	}else if (key == GLFW_KEY_I){
		drawAnchorPoints = false;
	}else if(key == GLFW_KEY_E){
		drawFrustums = true;
	}else if(key == GLFW_KEY_R){
		drawFrustums = false;
	}else if(key == GLFW_KEY_H){
		drawMainBB = true;
	}else if(key == GLFW_KEY_J){
		drawMainBB = false;
	}else if(key == GLFW_KEY_Z){
		printLoc = true;
	}else if(key == GLFW_KEY_G){
		exit(0);
	}else if(key == GLFW_KEY_C){
		if(PATHNUM < path.size()-1){
			PATHNUM +=1;
		}else{
			PATHNUM = 0;
		}
		cout << "PathNum: " << PATHNUM << "\n";
	}
 	


	if(drawFrustums == true){
		if(key == GLFW_KEY_T){
			if(drawF < Nodes.size()-1){
				drawF += 1;
			}else{
				drawF = 0;
			}
		}
		if(key == GLFW_KEY_Y){
			if(drawF > 0 ){
				drawF -= 1;
			}else{
				drawF = Nodes.size()-1;
			}
		}
	}
	wasdCam.updateCamera();
}

void createFBO(GLuint& fb, GLuint& tex) {
	//initialize FBO (global memory)
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	//set up framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	//set up texture
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		cout << "Error setting up frame buffer - exiting" << endl;
		exit(0);
	}
}

static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) {
		glfwGetCursorPos(window, &posX, &posY);
	}

}

static void cursor_callback(GLFWwindow *window, double posX, double posY)
{
	glfwGetCursorPos(window, &posX, &posY);

	if (firstMouse == 0) {
		curX = posX;
		curY = posY;
		firstMouse = 1;
	}

	double deltaX = posX - curX;
	double deltaY = curY - posY;

	curX = posX;
	curY = posY;

	beta += deltaX*(PI*2.2 / 800);
	alpha += deltaY*((PI / 2) / 1200);

	if (alpha > 89) {
		alpha = 89;
	}
	if (alpha < -89) {
		alpha = -89;
	}

	lookX = cos(alpha)*cos(beta);
	lookY = sin(alpha);
	lookZ = cos(alpha)*sin(beta);
	glm::vec3 lookAtPt = normalize(vec3(lookX, lookY, lookZ));
	lookAtPt += wasdCam.eye;
	wasdCam.updateLookAt(lookAtPt);
}

float p2wx(double in_x, float left) {
	float c = (-2*left)/(g_width-1.0);
	float d = left;
   return c*in_x+d;
}

float p2wy(double in_y, float bot) {
	//flip y
  	in_y = g_height -in_y;
	float e = (-2*bot)/(g_height-1.0);
	float f = bot;
   return e*in_y + f;
}

static void resize_callback(GLFWwindow *window, int width, int height) {
	g_width = width;
	g_height = height;
	glViewport(0, 0, width, height);
}

/* code to define the ground plane */
static void initGeom() {
   frustumObj.setUp(testCam.frustum);
   frustumObjTest.setUp(testCam.frustum);
}
	


static void init()
{
	GLSL::checkVersion();
	glfwGetFramebufferSize(window, &width, &height);
	// Set background color.
	glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

    // Initialize mesh.
   

    S = Obj(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_wreck_frag.glsl", "box.jpg", 0, 0,0);
	 S.initializeShape();

	 testS = Obj(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_wreck_frag.glsl", "box.jpg", 0, 0,0);
	 testS.addTransforms(vec3(-6,-2,1), vec3(4,8,4), 0, vec3(0,0,0));
	 testS.createBB();
	 testS.initializeShape();

	 anchor = Anchor(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_anchor_frag.glsl", "white.png", 1, 0,0);
	 anchor.initializeShape();

	 wreck = Obj(RESOURCE_DIR, "OldBoat.obj", "tex_vert.glsl", "tex_wreck_frag.glsl", "boattex.jpg", 2, 0,1);
	 wreck.addTransforms(vec3(0,0,0), vec3(3,3,3), 0, vec3(0,0,0));
	 wreck.createBB();
	 wreck.initializeShape();


	 boundingBox = Obj(RESOURCE_DIR, "cube.obj", "tex_vert.glsl", "tex_bb_frag.glsl", "pink.jpg", 3, 0,0);
	 boundingBox.initializeShape();

	 frustumObj = FrustumObj(RESOURCE_DIR, "cube.obj", "tex_vert.glsl", "tex_bb_frag.glsl", "blue.jpg", 4, 0,0);
	 frustumObjTest = FrustumObj(RESOURCE_DIR, "cube.obj", "tex_vert.glsl", "tex_bb_frag.glsl", "blue.jpg", 4, 0,0);




	// Initialize the GLSL programs

	S.initializeProg();
	testS.initializeProg();
	wreck.initializeProg();
	boundingBox.initializeProg();
	anchor.initializeProg();
	frustumObj.initializeProg();
	frustumObjTest.initializeProg();
	
	//////////////////////////////////////////////////////
   // Intialize textures
   //////////////////////////////////////////////////////   
 

   S.initializeTexture();
   testS.initializeTexture();
   wreck.initializeTexture();
   boundingBox.initializeTexture();
   anchor.initializeTexture();
   frustumObj.initializeTexture();
   frustumObjTest.initializeTexture();


	/// Add uniform and attributes to each of the programs

   S.addUniforms();
   testS.addUniforms();
   wreck.addUniforms();
   boundingBox.addUniforms();
   anchor.addUniforms();
   frustumObj.addUniforms();
   frustumObjTest.addUniforms();

}

void saveImage(int width, int height, int nbr){
    char buffer[50];
    sprintf(buffer,"image%d.png",nbr);
    GLubyte *data = new GLubyte[4*width*height];
    glReadPixels(0,0,width,height,GL_BGRA,GL_UNSIGNED_BYTE, data);
    cv::Mat imageMat(height, width, CV_8UC4, data);
    cv::flip(imageMat, imageMat, 0);
    cv::imwrite(buffer,imageMat);
    
}


/****DRAW
This is the most important function in your program - this is where you
will actually issue the commands to draw any geometry you have set up to
draw
********/
static void render()
{
	int rotCamIndex =0;;

	if(useCircularPath == true){
		CameraParser cp = CameraParser("cameraInfo.txt");
		cp.getInfo();
		//rotating Camera
		float rotCamRadius = cp.camRadius;
		float rotCamHeight = cp.camHeight;
		rotCam.updateSpeed(cp.camSpeed);
		float rotCamX = (sin(glfwGetTime()/rotCam.camSpeed) * rotCamRadius); //glfwGetTime()
		float rotCamZ = (cos(glfwGetTime()/rotCam.camSpeed) * rotCamRadius);
		vec3 rotCamPos = vec3(rotCamX,rotCamHeight,rotCamZ);
		rotCam.updatePosition(rotCamPos);
		rotCam.updateLookAt(cp.lookAt);

		Anchor newNode = Anchor();
		Camera c = Camera(rotCam.eye, rotCam.lookAt, 1);
	    c.updatePerspective(aspect, 45, zNear, 90);
	    c.setRayParameters(1,200);
	    c.createFrustum();
		cout << "Weight: " << newNode.weight << "\n";

	}

	//camera 
	//have method that updates these camera things 

	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	aspect = width/(float)height;
	glViewport(0, 0, width, height);

	//give cameras P information
	//rotCam.updatePerspective(aspect,FOV,zNear,zFar);

	//Matrices 
	auto P = make_shared<MatrixStack>();
	auto M = make_shared<MatrixStack>();
	mat4 V;
	

   P->pushMatrix();
   P->perspective(FOV, aspect, zNear, zFar);
   //set frame buffer to [0] 
   glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[0]);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //now draw normal scene to screen 
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
   //set camera view
    if(useLawnMowerPath == true || useCircularPath == true || useNodePath == true){
	   	if(useLawnMowerPath == true){
	   		V = lawnmower[lawnmowerIndex].cameraView;
	   		currentCam = lawnmower[lawnmowerIndex];
	   		maxCamIndex = lawnmower.size();
	   		lawnmowerIndex++;
	   	}else if(useCircularPath == true){
	   		V = rotCam.cameraView;
	   		currentCam = rotCam;
	   		maxCamIndex  = 700;
	   		
	   	}else if(useNodePath == true){
	   		V = path[PATHNUM].camera.cameraView;
	   		//V = path[PATHNUM].camera.cameraView;
	   		currentCam = path[PATHNUM].camera;
	   		maxCamIndex = path.size()-1;
	   		if(PATHNUM < path.size()-1){
	   			PATHNUM++;
	   		}else{
	   			PATHNUM =0;
	   		}
	   	}
	   	if(camIndex > maxCamIndex && useNodePath == false){
	   		exit(0);
	   	}
	   	camIndex ++;
	}else{
		currentCam = wasdCam;
	    V = wasdCam.cameraView;
	}



	wreck.draw(M,P,V);
	//testS.draw(M,P,V);

   //TESTING
  
  /*   S.addTransforms(vec3(0,4,6), vec3(1,1,1), 0, vec3(0,0,0));
    S.draw(M,P,V);
    S.addTransforms(vec3(0,0,0), vec3(1,1,1), 0, vec3(0,0,0));
  	 S.draw(M,P,V);
  	 frustumObjTest.draw(M,P,V);

  	 for(int i =0 ; i < testAnchor.camera.lookAts.size() -1 ; i ++){ //-1
  	 	S.addTransforms(testAnchor.camera.lookAts[i], vec3(.5,.5,.5), 0, vec3(0,0,0));
    	S.draw(M,P,V);
  	 }*/
  	 


	if(drawBoundingBoxes == true){

		for(int i = 0; i < sceneObjects.size(); i ++){
			boundingBox.addTransforms(sceneObjects[i].BBtrans,sceneObjects[i].BBscale, 0, vec3(0,0,0));
			boundingBox.draw(M,P,V);
		}
	}

	if(drawMainBB == true){
		boundingBox.addTransforms(globalBBtrans,globalBBscale, 0, vec3(0,0,0));
		boundingBox.draw(M,P,V);
	}
   
   if(drawGrid == true){
	  for(int i =0; i < grid.size(); i++){
	  	   grid[i].draw(M,P,V);
	  }
	}

	 if(drawNodes == true){
	  for(int i =0; i < Nodes.size(); i++){
	  	   Nodes[i].draw(M,P,V);
	  }
	}

   if(drawAnchorPoints == true){
	  for(int i =0; i < anchorPts.size(); i++){
	  	   anchorPts[i].draw(M,P,V);
	  }
	}

	if(drawFrustums == true){ //Nodes for those that intersect and AnchorPoints for all 
		Nodes[drawF].draw(M,P,V);
		Nodes[drawF].fObj.draw(M,P,V);
	}


    if(printLoc == true){
    	cout << "Cam LOC: " << currentCam.eye.x << " " << currentCam.eye.y << " " << currentCam.eye.z << "\n";
    	printLoc = false;
    }
	
	if(camIndex%3 == 0 && camIndex > 0){
   		saveImage(width,height,imageNum);
   		imageNum++;
   	}
    
}


void selectRandomCameras(int sampleNums, int strat){
   if(strat == 10 || strat < 0){
   	int grpSize = grid.size()/10;
   	for(int i =0; i < sampleNums /10; i++){

   		int temp0 = rand() % grpSize;
   		int temp1 = rand() % grpSize + grpSize;
   		int temp2 = rand() % grpSize + 2*grpSize;
   		int temp3 = rand() % grpSize + 3*grpSize;
   		int temp4 = rand() % grpSize + 4*grpSize;
   		int temp5 = rand() % grpSize + 5*grpSize;
   		int temp6 = rand() % grpSize + 6*grpSize;
   		int temp7 = rand() % grpSize + 7*grpSize;
   		int temp8 = rand() % grpSize + 8*grpSize;
   		int temp9 = rand() % grpSize + 9*grpSize;

   		//cout << "nums: " << temp0 << " " << temp1 << " " << temp2 << " " << temp3 << " " << temp4 << " " << temp5 << " " << temp6 << " " << temp7 << " " << temp8 << " " << temp9 << "\n";

   		anchorPts.push_back(grid[temp0]);
   		anchorPts.push_back(grid[temp1]);
   		anchorPts.push_back(grid[temp2]);
   		anchorPts.push_back(grid[temp3]);
   		anchorPts.push_back(grid[temp4]);
   		anchorPts.push_back(grid[temp5]);
   		anchorPts.push_back(grid[temp6]);
   		anchorPts.push_back(grid[temp7]);
   		anchorPts.push_back(grid[temp8]);
   		anchorPts.push_back(grid[temp9]);

   		grid[temp0].updateAnchorType(0);
   		grid[temp1].updateAnchorType(0);
   		grid[temp2].updateAnchorType(0);
   		grid[temp3].updateAnchorType(0);
   		grid[temp4].updateAnchorType(0);
   		grid[temp5].updateAnchorType(0);
   		grid[temp6].updateAnchorType(0);
   		grid[temp7].updateAnchorType(0);
   		grid[temp8].updateAnchorType(0);
   		grid[temp9].updateAnchorType(0);

   	}
   }
   cout << "anchor size: " << anchorPts.size() << "\n";
   for(int i =0; i < anchorPts.size(); i++){
   	anchorPts[i].updateAnchorType(0);
   }
}


void setCameras(){
	for(int i =0; i < anchorPts.size(); i++){
		vec3 lookAtPt;
		anchorPts[i].createLookAt(anchorPts[i].pos,&lookAtPt);
		//cout << "origin: " << anchorPts[i].pos.x << " " << anchorPts[i].pos.y << " " << anchorPts[i].pos.z <<"\n";
		//cout << "lookAt: " << lookAtPt.x << " " << lookAtPt.y << " " << lookAtPt.z <<"\n";
		Camera c  = Camera(anchorPts[i].pos,lookAtPt,0);
		c.updatePerspective(aspect, 45, zNear, zFar);
		c.setRayParameters(1,200);
		c.createFrustum();
		anchorPts[i].setCamera(c,c.startPos.y);
		frustumObj.setUp(anchorPts[i].camera.frustum);
		anchorPts[i].fObj = frustumObj;
	}
}

bool inBB(vec3 loc, vec3 min, vec3 max){
	int x,y,z =0;
	if(loc.x < max.x && loc.x > min.x){
		x=1;
	}
	if(loc.y < max.y && loc.y > min.y){
		y=1;
	}
	if(loc.z < max.z && loc.z > min.z){
		z=1;
	}
	if(x == 1 && y ==1 && z ==1){
		return true;
	}
}

bool boxFrustumIntersectionTest(Frustum fru)
{
    int out;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].x > globalBB.max.x)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].x < globalBB.min.x)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].y > globalBB.max.y)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].y < globalBB.min.y)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].z > globalBB.max.z)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].z < globalBB.min.z)?1:0); if( out==8 ) return false;

    return true;
}

void checkForBBFrustumInterestion(){
	for (int i =0; i < anchorPts.size(); i++){
		//0 is flase 
		 bool hit = false;
		 hit = boxFrustumIntersectionTest(anchorPts[i].camera.frustum);
		 if(hit == true){
		 	anchorPts[i].updateAnchorType(hit);
		 	Nodes.push_back(anchorPts[i]);
		 }
	}
}

void getAnchorCameraWeights(){
	float weightTotal = 0;
	float SD =0;

	cout<< "nodes size: " << Nodes.size() << "\n";
	for (int i = 0; i < Nodes.size(); i++){
			cout << "Roadmap size: " << roadMap.size() << "\n";
		 	Nodes[i].getWeight(globalBB,hits, roadMap);
		 	weightTotal += Nodes[i].weight;
	} 	
  	nodeWeightMean = weightTotal/Nodes.size();
  	for (int i = 0; i < Nodes.size(); i++){
  		SD += pow(Nodes[i].weight - nodeWeightMean, 2);
  	}
    SD = sqrt(SD/Nodes.size());
    highLevelCutOff = SD + nodeWeightMean;
	for (int i =0; i < Nodes.size(); i++){
		 	if(Nodes[i].weight >= highLevelCutOff){
		 		highestWeightNodes.push_back(Nodes[i]);

		 	}
	}
}

float delta(float a, float b){
	return abs(a-b);
}

void printHits(Anchor newNode){
	if ((newNode.hitting & 16) > 0) {
		cout << "P1: 1\n";
	}
	else {
		cout << "P1: 0\n";
	}
	if ((newNode.hitting & 8) > 0) {
		cout << "P2: 1\n";
	}
	else {
		cout << "P2: 0\n";
	}
	if ((newNode.hitting & 4) > 0) {
		cout << "P3: 1\n";
	}
	else {
		cout << "P3: 0\n";
	}
	if ((newNode.hitting & 2) > 0) {
		cout << "P4: 1\n";
	}
	else {
		cout << "P4: 0\n";
	}
	if ((newNode.hitting & 1) > 0) {
		cout << "P5: 1\n";
	}
	else {
		cout << "P5: 0\n";
	}
	

}

int generateNewNode(int numNodes){
   Anchor curr;
   Anchor newNode = Anchor();
   int nodeIndex = -1;
   int currIdx;
   bool high = false;

  	/* For each newNode generated, have every other one expand off of current nodes in the */
    if(iteration % 2 == 0){
    	currIdx = rand() % highWeightNodes.size();
		curr = highWeightNodes[currIdx];
		high = true;
    }else{ //chose a random node in roadmap untill nodes weight is > weightThreshold 
	   	nodeIndex = rand() % roadMap.size();
		curr = roadMap.at(nodeIndex);
	}

	newNode.parentIndex = curr.ndex;
	//creates a random anchor point to expand off of
	//cout << "creating new node\n";
	newNode.createAnchor(pitchIteration, &curr, numNodes, curr.pathLength + 1, aspect, zNear, globalBB, hits, roadMap);
	cout<< "high level: " << highLevelCutOff << "\n";
	cout<< "newNode Weight: " << newNode.weight << "\n";
  
   

    newNode.ndex = roadMap.size();
	roadMap.push_back(newNode);

	if (curr.weight < highWeightAvg && high == true) {
		highWeightNodes.erase(highWeightNodes.begin() + currIdx);
		cout << "REMOVED \n";
		//highWeightAvg = (((highWeightAvg * (highWeightNodes.size() + 1)) - curr.weight) / highWeightNodes.size());
	}

	if(newNode.weight > highLevelCutOff){
		highWeightNodes.push_back(newNode);
		cout << "HWN size : " << highWeightNodes.size() << "\n";
		highWeightAvg = (((highWeightAvg * (highWeightNodes.size() - 1)) + newNode.weight) / highWeightNodes.size());
		cout << "Average: " << highWeightAvg << "\n";
		highLevelCutOff = 0.6 * highWeightAvg;
		cout << "new node added to map: " << roadMap.size() << "\n";
	}
	cout << "Average: " << highWeightAvg << "\n";


   iteration ++;
   pitchIteration ++;
   
   //cout << "completed\n";
	cout << "pathLength: " <<newNode.pathLength << "\n";   //WE HAVE A COMPLETE PATH
	
	//if (newNode.pathLength == numNodes) {

	printHits(newNode);

	if (newNode.hitting >= 31) {
		cout<<"DONE\n";

		if (debug) {
			double avgRoadmap = 0;
			for (Anchor mapNode : roadMap) {
				avgRoadmap += mapNode.weight;
			}
			std::cout << "roadMap size: " << roadMap.size() << std::endl;
			std::cout << "avg roadmap node weight: " 
				<< avgRoadmap / roadMap.size() << std::endl;
		}
 
		// Pull actual path off roadmap from current node up to root node
		double avgPathWeight = 0;

		//cout<<"path size before: " << path.size() << "\n";
      int p =0;
		do{
			path.push_back(newNode);
			//cout<<"path size: " << path.size() << "\n";
			avgPathWeight += newNode.weight;
			//cout << "curr index: " << newNode.ndex << "\n";
			//cout << "parent index: " << newNode.parentIndex << "\n";
			newNode = roadMap[newNode.parentIndex];
			cout<< "is it a root? " << newNode.root << "\n";
			p++;

		}while(newNode.root != 1);

		cout<<"path size: " << path.size() << "\n";

		//We built path from the leaf up to root so reverse it
		std::reverse(path.begin(), path.end());
//
		avgPathWeight /= path.size();
		std::cout << "avg path weight: " << avgPathWeight << std::endl;
   
		if (debug) {
			std::cout << "\npercent in high weight list: " 
				<< (double)highestWeightNodes.size() / roadMap.size() << std::endl;
		}


		// Write path to file
		std::ofstream outfile(RESOURCE_PATH "path.txt");
		if (outfile.is_open()) {
			outfile << path.size() << std::endl;
			for (int i = 0; i < path.size(); i++) {
				outfile << path[i].pos.x << " " << path[i].pos.y << " " << path[i].pos.z << " " 
						<< path[i].camera.lookAt.x << " " << path[i].camera.lookAt.y  << " " << path[i].camera.lookAt.z << std::endl;
			}
			// Write road map size, average node weight in path, 
			// and time to generate path to file
			outfile << roadMap.size() << std::endl;
			outfile << avgPathWeight << std::endl;
		}
		else {
			std::cerr << "Unable to open file" << std::endl;
		}
		outfile.close();
		return 1;
	}
	return 0;

}

int main(int argc, char **argv)
{
	srand ( time(0) );
	rand();
	g_width = 1200;
	g_height = 800;
	aspect = g_width/g_height;
	/* we will always need to load external shaders to set up where */
	if(argc < 3) {
      cout << "Please specify the resource directory and camera type" << endl;
      return 0;
   }
   RESOURCE_DIR = argv[1] + string("/");
   cameraType = atoi(argv[2]); 

   if(cameraType == 1){
   	useLawnMowerPath = true;
   }else if(cameraType == 2){
   	useCircularPath = true;
   }else if(cameraType == 3){
   	useNodePath = true;
   }

	/* your main will always include a similar set up to establish your window
      and GL context, etc. */

	// Set error callback as openGL will report errors but they need a call back
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	//request the highest possible version of OGL - important for mac
	
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(g_width, g_height, "textures", NULL, NULL);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}

	glGetError();
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	//set the window resize call back
	glfwSetFramebufferSizeCallback(window, resize_callback);
	//set the mouse call back
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetCursorPosCallback(window, cursor_callback);


	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	cout << "aspect in main: " << aspect << "\n";

   //TEST STUFF
	testCam.updatePerspective(aspect,20.0f,zNear,100);
	testCam.setRayParameters(1,200);
	testCam.createFrustum();
	frustumObjTest.setVertices(testCam.frustum);

	init();
	cout << "done initializing shaders" << endl;
	initGeom();
	cout << "done initializing geometry" << endl;


   //add important wrecks to sceneObj vector

	sceneObjects.push_back(wreck);
	//sceneObjects.push_back(testS); //testing one 

		//make globalBB
	 for(int i = 0; i != sceneObjects.size () ; i++){
	 	globalBB.addBoundingBox(sceneObjects[i].bb);
	 }

	 globalBB.setPlanes();

	//make globalBB
	 for(int i = 0; i != sceneObjects.size () ; i++){
	 	globalBB.addBoundingBox(sceneObjects[i].bb);
	 }


	cout << "global min: " << globalBB.min.x << " " << globalBB.min.y << " " << globalBB.min.z << "\n"; 
	cout << "global max: " << globalBB.max.x << " " << globalBB.max.y << " " << globalBB.max.z << "\n"; 
	globalBBscale = vec3((globalBB.max.x - globalBB.min.x)/2.0,(globalBB.max.y - globalBB.min.y)/2.0,(globalBB.max.z - globalBB.min.z)/2.0);	
    globalBBtrans = vec3(avg(globalBB.min.x,globalBB.max.x),avg(globalBB.min.y,globalBB.max.y),avg(globalBB.min.z,globalBB.max.z));

    //TEST CONTINUED 
    //testAnchor.camera = testCam; 
    //cout << "Roadmap size: " << roadMap.size() << "\n";
	//testAnchor.getWeight(globalBB, hits, roadMap);
   
	//testAnchor.checkBBPlanes(Ray(vec3(-100,10,0),vec3(1,0,0)),globalBB,hits, roadMap);

	//USING LAWNMOWERPATH
	if(useLawnMowerPath == true) { //cameraType == 0){
		vec3 minBB = globalBB.min * lawnMowerSizeMin;
		vec3 maxBB = globalBB.max * lawnMowerSizeMax;
		float y = maxBB.y+1;
		float xIncrement = (maxBB.x-minBB.x)/6;
		for(int i =0; i <=6 ; i += 2){
			for(float z =0; z <=(maxBB.z-minBB.z); z+= (maxBB.z-minBB.z)/700){
				Camera c = Camera(vec3(minBB.x+i*xIncrement,y,minBB.z+z),vec3(minBB.x+i*xIncrement+.1,0,minBB.z+z+.1),1);//vec3(globalBB.min.x+x,y+.5,globalBB.min.z+z),1);
				lawnmower.push_back(c);
			}
			for(float z = maxBB.z; z >= minBB.z ; z-= (maxBB.z-minBB.z)/700){
				Camera c = Camera(vec3(minBB.x+((i+1)*xIncrement),y,z),vec3(minBB.x+(i+1)*xIncrement-.1,0,z-.1),1);//vec3(globalBB.min.x+x,y+.5,globalBB.min.z+z),1);
				lawnmower.push_back(c);
			}
		}
		

	}

	//USING NODE PATH 
	if(useNodePath == true || cameraType == 0){
		vec3 minBB = globalBB.min * sceneSizeMin;
		vec3 maxBB = globalBB.max * sceneSizeMax;
	   //get sample box information
	   sampleDelta = 10;
	   for(float i = minBB.x; i <= maxBB.x; i += sampleDelta){
	      for(float j = minBB.y; j <= maxBB.y; j+= sampleDelta){
	        for(float k= minBB.z; k <= maxBB.z; k+= sampleDelta){
	         	vec3 tempLoc = vec3(i,j,k);
	         	if(inBB(tempLoc, globalBB.min*distanceFromWreck, globalBB.max*distanceFromWreck) == false){
		         	anchor.addPosition(tempLoc);
		         	anchor.addTransforms(tempLoc, vec3(1,1,1), 0, vec3(0,0,0));
		         	grid.push_back(anchor);
		         }
	         }
	      }
	   }
	   cout << "grid size: " << grid.size() << "\n";
	   

	   selectRandomCameras(grid.size()/2, stratifiedNum);
	   setCameras();
	   checkForBBFrustumInterestion();
	   getAnchorCameraWeights();
	   
	   //START MAKING MAP!!! 
	 	
	   //randomly selects a high weight node to use as root
	   if(highestWeightNodes.size() > 0){
		   int root = rand() % highestWeightNodes.size();
		   rootNode = highestWeightNodes[root];
		   //This gives the rootnode a parent for going throught the list later. 
		   rootNode.root =1;
		   rootNode.pathLength = 0;
		   rootNode.parentIndex = -1;
		   //sets rootNode in road map
		   rootNode.ndex =0;
		   roadMap.push_back(rootNode);
		   highWeightNodes.push_back(rootNode);
		   highWeightAvg = rootNode.weight;

		   int maxNodes =20;
		   int currNodes = 1;

		   while(generateNewNode(maxNodes-1) == 0){
		       //cout << "curNode: " << currNodes << "\n";   
		   	   currNodes++;
		   	   for(int i =0; i < 5; i++){
	 					//cout << i<< ": " << hits[i] <<"\n";
	 		   }
	 		   //resetHits();
		   }
		}
	}



	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
