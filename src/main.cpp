/* Base code for texture mapping lab */
/* includes three images and three meshes - Z. Wood 2016 */
#define GLFW_INCLUDE_NONE

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include <unordered_map>
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
#include "Voxel.h"
#include "TextureKole.hpp"
#include "Framebuffer.hpp"
#include "Image.hpp"
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
vec3 distanceFromWreck = vec3(1.2);

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
int highestWeightNodeIndex = 0; 
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
BoundingBox wreckBB = BoundingBox();

vector<Camera> lawnmower; 

//rendering options
bool drawBoundingBoxes = false;
bool drawAnchorPoints = false;
bool drawGrid = false;
bool drawSDS = false;
bool drawNodes = false;
bool drawPath = false;
bool drawFrustums = false;
bool drawMainBB = false;
bool printLoc = false;
bool debug = true;
bool showPath = false;
bool useLawnMowerPath = false;
bool useCircularPath = false;
bool useNodePath = false;
bool limitByTime = false;
bool limitByPathLength = false;
bool limitByViews = false;
bool validate = false;
string pathName = "";
vec3 validationCell;

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

vec3 wreckBBtrans, wreckBBscale;

// Spatial Data Structure
vec3 realMin = vec3(1.1754E+38F);
vec3 realMax = vec3(-1.1754E+38F);
float xRealExtent = 0.0f;
float yRealExtent = 0.0f;
float zRealExtent = 0.0f;
float dimSDSx = 0.0f;
float dimSDSy = 0.0f;
float dimSDSz = 0.0f;
float voxelSize = 0.0f; // currently use max extent

struct KeyFuncs
{
    size_t operator()(const vec3& k)const
    {
        return std::hash<float>()(k.x) + std::hash<float>()(k.y) + std::hash<float>()(k.z);
    }

    bool operator()(const vec3& a, const vec3& b)const
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};


typedef unordered_map<vec3, Voxel, KeyFuncs, KeyFuncs> SpatialDataStructure; // hasmap representing 3D grid of voxels, voxels contain PRM nodes
SpatialDataStructure grid3D;
vector<Anchor> validationGrid;

void computeSpatialBB(BoundingBox targetBB)
{
    vec3 tempMin = realMin * 2;
    vec3 tempMax = realMax * 2;

    if (targetBB.min.x < tempMin.x)
    {
        tempMin.x = targetBB.min.x;
    }
    if (targetBB.min.y < tempMin.y)
    {
        tempMin.y = targetBB.min.y;
    }
    if (targetBB.min.z < tempMin.z)
    {
        tempMin.z = targetBB.min.z;
    }

    if (targetBB.max.x > tempMax.x)
    {
        tempMax.x = targetBB.max.x;
    }
    if (targetBB.max.y > tempMax.y)
    {
        tempMax.y = targetBB.max.y;
    }
    if (targetBB.max.z > tempMax.z)
    {
        tempMax.z = targetBB.max.z;
    }
    
    // Compute extents
    float xTempExtent = tempMax.x - tempMin.x;
    float yTempExtent = tempMax.y - tempMin.y;
    float zTempExtent = tempMax.z - tempMin.z;

    // Compute real target min & max
    realMin.x = tempMin.x - 0.5f * xTempExtent;
    realMin.y = tempMin.y - 0.5f * yTempExtent;
    realMin.z = tempMin.z - 0.5f * zTempExtent;

    realMax.x = tempMax.x + 0.5f * xTempExtent;
    realMax.y = tempMax.y + 0.5f * yTempExtent;
    realMax.z = tempMax.z + 0.5f * zTempExtent;

    xRealExtent = realMax.x - realMin.x;
    yRealExtent = realMax.y - realMin.y;
    zRealExtent = realMax.z - realMin.z;

    voxelSize = 1.0f;  //maxExtent, 1/2 speed auv travels
    // std::cout << "Voxel size: " << voxelSize << std::endl;

    // set x, y, and z dimensions of data structure
    dimSDSx = xRealExtent / voxelSize + 1;
    dimSDSy = yRealExtent / voxelSize + 1;
    dimSDSz = zRealExtent / voxelSize + 1;

    //std::cout << "Extent x: " << xRealExtent << std::endl;
    //std::cout << "Extent y: " << yRealExtent << std::endl;
    //std::cout << "Extent z: " << zRealExtent << std::endl;

    //std::cout << "BB min x: " << realMin.x << std::endl;
    //std::cout << "BB max x: " << realMax.x << std::endl;
    //std::cout << "BB min y: " << realMin.y << std::endl;
    //std::cout << "BB max y: " << realMax.y << std::endl;
    //std::cout << "BB min z: " << realMin.z << std::endl;
    //std::cout << "BB max z: " << realMax.z << std::endl;
}

float getMaxExtent()
{
    return std::max(std::max(xRealExtent, yRealExtent), zRealExtent);
}


bool inBB(vec3 loc, vec3 min, vec3 max)
{
    int x = 0;
    int y = 0;
    int z = 0;

    if (loc.x < max.x && loc.x > min.x)
    {
        x = 1;
    }
    if (loc.y < max.y && loc.y > min.y)
    {
        y = 1;
    }
    if (loc.z < max.z && loc.z > min.z)
    {
        z = 1;
    }

    if (x == 1 && y == 1 && z == 1)
    {
        return true;
    }

    return false;
}

// can only be called after voxel size is set in initSpatialDS
vec3 worldToIndexCoords(vec3 worldCoords)
{
    vec3 indexCoords = vec3(0.0f);

    /*std::cout << "World Coord X: " << worldCoords.x << std::endl;
    std::cout << "World Coord Y: " << worldCoords.y << std::endl;
    std::cout << "World Coord Z: " << worldCoords.z << std::endl;*/

    indexCoords.x = std::floor((worldCoords.x - realMin.x) / voxelSize);
    indexCoords.y = std::floor((worldCoords.y - realMin.y) / voxelSize);
    indexCoords.z = std::floor((worldCoords.z - realMin.z) / voxelSize);

    /*  std::cout << "Index Coord X: " << indexCoords.x << std::endl;
      std::cout << "Index Coord Y: " << indexCoords.y << std::endl;
      std::cout << "Index Coord Z: " << indexCoords.z << std::endl;*/

    return indexCoords;
}

vec3 indexToWorldCoords(vec3 indexCoords)
{
    vec3 worldCoords = vec3(0);

    /*  std::cout << "Index Coord X: " << indexCoords.x << std::endl;
       std::cout << "Index Coord Y: " << indexCoords.y << std::endl;
       std::cout << "Index Coord Z: " << indexCoords.z << std::endl;*/

    indexCoords.x = std::floor((worldCoords.x - realMin.x) / voxelSize);
    indexCoords.y = std::floor((worldCoords.y - realMin.y) / voxelSize);
    indexCoords.z = std::floor((worldCoords.z - realMin.z) / voxelSize);

    worldCoords.x = indexCoords.x * voxelSize + realMin.x;
    worldCoords.y = indexCoords.y * voxelSize + realMin.y;
    worldCoords.z = indexCoords.z * voxelSize + realMin.z;
    
     /*std::cout << "World Coord X: " << worldCoords.x << std::endl;
     std::cout << "World Coord Y: " << worldCoords.y << std::endl;
     std::cout << "World Coord Z: " << worldCoords.z << std::endl;*/

    return worldCoords;
}

//// set up spatial data structure (3D grid of voxels)
//static void initSpatialDS(BoundingBox targetBB)
//{
//    computeSpatialBB(targetBB);
//
//    voxelSize = 1.0f;  //maxExtent, 1/2 speed auv travels
//    // std::cout << "Voxel size: " << voxelSize << std::endl;
//
//    // set x, y, and z dimensions of data structure
//    int dimX = xRealExtent / voxelSize + 1;
//    int dimY = yRealExtent / voxelSize + 1;
//    int dimZ = zRealExtent / voxelSize + 1;
//  /*  std::cout << "Dim x: " << dimX << std::endl;
//    std::cout << "Dim y: " << dimY << std::endl;
//    std::cout << "Dim z: " << dimZ << std::endl;*/
//
//    vector<vector<vector<Voxel>>> tempGrid(dimX, vector<vector<Voxel>>(dimY, vector<Voxel>(dimZ)));
//    
//    // fill in 3D grid with voxels
//    for (int x = 0; x < dimX; x++)
//    {
//        for (int y = 0; y < dimY; y++)
//        {
//            for (int z = 0; z < dimZ; z++)
//            {
//                tempGrid[x][y][z] = Voxel();
//            }
//        }
//    }
//
//    grid3D = tempGrid;
//}

void createValidationGrid()
{
    unsigned char* buffer = new unsigned char[xRealExtent*zRealExtent];
    for (int x = realMin.x; x < realMax.x; x++)
    {
        for (int z = realMin.z; z < realMax.z; z++)
        {
            vec3 tempLoc = vec3(x, validationCell.y, z);
            if (inBB(tempLoc, wreckBB.min, wreckBB.max) == false)
            {
                if (x % 15 == 0 && z % 15 == 0)
                {
                    //  vec3 pos = indexToWorldCoords(vec3(tempLoc));
                    anchor.addPosition(tempLoc);
                    //  anchor.pos = pos;
                    anchor.addTransforms(tempLoc, vec3(1, 1, 1), 0, vec3(1, 1, 1));
                    validationGrid.push_back(anchor);
                }
            }
            //else
            //{
            //    anchor.weight = -1.0f;
            //    anchor.addPosition(tempLoc);
            //    //  anchor.pos = pos;
            //    anchor.addTransforms(tempLoc, vec3(1, 1, 1), 0, vec3(1, 1, 1));
            //    validationGrid.push_back(anchor);
            //}
        }
    }
}

void insertIntoSpatialDS(Anchor node)
{
    const vec3 spatialIndex = worldToIndexCoords(node.pos);
    vector<Anchor> curNodes = grid3D[spatialIndex].getPRMNodes();
    curNodes.push_back(node);
    grid3D[spatialIndex].setPRMNodes(curNodes);
}

Anchor getNodeFromBinWithLeast()
{
    Voxel smallestBin;
    int least = INT_MAX;
    int count = 0;

    for (auto it = grid3D.begin(); it != grid3D.end(); it++)
    {
        Voxel curBin = it->second.getPRMNodes();
        int numNodes = curBin.getPRMNodes().size();

        if (numNodes > 0)
        {
            count++;
        }

        if (numNodes < least && numNodes > 0)
        {
            least = numNodes;
            smallestBin = curBin;
        }
    }

    float space = float(dimSDSx * dimSDSy * dimSDSz);
    cout << "Percent of Space Occupied: " << float(count) / space << endl;
    int nodeIndex = rand() % smallestBin.getPRMNodes().size();
    return smallestBin.getPRMNodes().at(nodeIndex);
}

float avg(float num1, float num2)
{
    return (num1 + num2) / 2.0;
}

static void error_callback(int error, const char *description)
{
    cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
	     glfwSetWindowShouldClose(window, GL_TRUE);
    }
    //move camera 
    else if (key == GLFW_KEY_S)
    {
	     wasdCam.eye += wasdCam.camSpeed*wasdCam.vecW;
	     wasdCam.lookAt += wasdCam.camSpeed*wasdCam.vecW;
    }
    else if (key == GLFW_KEY_A)
    {
	     wasdCam.eye -= wasdCam.camSpeed*wasdCam.vecU;
	     wasdCam.lookAt -= wasdCam.camSpeed*wasdCam.vecU;
    }
    else if (key == GLFW_KEY_W)
    {
	     wasdCam.eye -= wasdCam.camSpeed*wasdCam.vecW;
	     wasdCam.lookAt -= wasdCam.camSpeed*wasdCam.vecW;
    }
    else if (key == GLFW_KEY_D)
    {
	     wasdCam.eye += wasdCam.camSpeed*wasdCam.vecU;
	     wasdCam.lookAt += wasdCam.camSpeed*wasdCam.vecU;
    }
    else if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
	     drawBoundingBoxes = !drawBoundingBoxes;
    }
    else if (key == GLFW_KEY_K && action == GLFW_PRESS)
    {
	     drawGrid = !drawGrid;
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        drawSDS = !drawSDS;
    }
    else if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
	     drawNodes = !drawNodes;
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
	     drawPath = !drawPath;
    }
    else if (key == GLFW_KEY_U && action == GLFW_PRESS)
    {
	     drawAnchorPoints = !drawAnchorPoints;
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
	     drawFrustums = !drawFrustums;
    }
    else if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
	     drawMainBB = !drawMainBB;
    }
    else if (key == GLFW_KEY_Z)
    {
	     printLoc = true;
    }
    else if (key == GLFW_KEY_G)
    {
	     exit(0);
    }
    else if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
	     if (PATHNUM < path.size() - 1)
	     {
	         PATHNUM++;
	     }
	     else
	     {
            PATHNUM = 0;
	     }
	     cout << "PathNum: " << PATHNUM << "\n";
    }

    if (drawFrustums == true)
    {
	     if (key == GLFW_KEY_T && action == GLFW_PRESS)
        {
	         if (drawF < Nodes.size() - 1)
            {
		          drawF++;
		      }
            else
            {
		          drawF = 0;
		      }
	     }
	     if (key == GLFW_KEY_Y && action == GLFW_PRESS)
        {
		      if (drawF > 0)
            {
		          drawF--;
		      }
            else
            {
		          drawF = Nodes.size() - 1;
		      }
	     }
    }
    wasdCam.updateCamera();
}

void createFBO(GLuint& fb, GLuint& tex)
{
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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        cout << "Error setting up frame buffer - exiting" << endl;
        exit(0);
    }
}

static void mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
	     glfwGetCursorPos(window, &posX, &posY);
    }
}

static void cursor_callback(GLFWwindow *window, double posX, double posY)
{
    glfwGetCursorPos(window, &posX, &posY);

    if (firstMouse == 0)
    {
	     curX = posX;
	     curY = posY;
	     firstMouse = 1;
    }

    double deltaX = posX - curX;
    double deltaY = curY - posY;

    curX = posX;
    curY = posY;

    beta += deltaX * (PI * 2.2 / 800);
    alpha += deltaY * ((PI / 2) / 1200);

    if (alpha > 89)
    {
	     alpha = 89;
    }
    if (alpha < -89)
    {
	     alpha = -89;
    }

    lookX = cos(alpha) * cos(beta);
    lookY = sin(alpha);
    lookZ = cos(alpha) * sin(beta);
    glm::vec3 lookAtPt = normalize(vec3(lookX, lookY, lookZ));
    lookAtPt += wasdCam.eye;
	wasdCam.updateLookAt(lookAtPt);
}

float p2wx(double in_x, float left)
{
	float c = (-2 * left) / (g_width - 1.0);
	float d = left;
   return c * in_x + d;
}

float p2wy(double in_y, float bot)
{
    // flip y
    in_y = g_height - in_y;
    float e = (-2 * bot) / (g_height - 1.0);
    float f = bot;
    return e * in_y + f;
}

static void resize_callback(GLFWwindow *window, int width, int height)
{
    g_width = width;
    g_height = height;
    glViewport(0, 0, width, height);
}

// define the ground plane
static void initGeom()
{
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
    S = Obj(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_wreck_frag.glsl", "box.jpg", 0, 0, 0);
    S.initializeShape();

    testS = Obj(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_wreck_frag.glsl", "box.jpg", 0, 0, 0);
    testS.addTransforms(vec3(-6, -2, 1), vec3(4, 8, 4), 0, vec3(0, 0, 0));
    testS.createBB();
    testS.initializeShape();

    anchor = Anchor(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_anchor_frag.glsl", "white.png", 1, 0, 0);
    anchor.initializeShape();

    wreck = Obj(RESOURCE_DIR, "OldBoat.obj", "tex_vert.glsl", "tex_wreck_frag.glsl", "boattex.jpg", 2, 0, 1);
    wreck.addTransforms(vec3(0, 0, 0), vec3(3, 3, 3), 0, vec3(0, 0, 0));
    wreck.createBB();
    wreck.initializeShape();

    boundingBox = Obj(RESOURCE_DIR, "cube.obj", "tex_vert.glsl", "tex_bb_frag.glsl", "pink.jpg", 3, 0, 0);
    boundingBox.initializeShape();

    frustumObj = FrustumObj(RESOURCE_DIR, "cube.obj", "tex_vert.glsl", "tex_bb_frag.glsl", "blue.jpg", 4, 0, 0);
    frustumObjTest = FrustumObj(RESOURCE_DIR, "cube.obj", "tex_vert.glsl", "tex_bb_frag.glsl", "blue.jpg", 4, 0, 0);

    // Initialize the GLSL programs
    S.initializeProg();
    testS.initializeProg();
    wreck.initializeProg();
    boundingBox.initializeProg();
    anchor.initializeProg();
    frustumObj.initializeProg();
    frustumObjTest.initializeProg();
	
    // Intialize textures
    S.initializeTexture();
    testS.initializeTexture();
    wreck.initializeTexture();
    boundingBox.initializeTexture();
    anchor.initializeTexture();
    frustumObj.initializeTexture();
    frustumObjTest.initializeTexture();

    // Add uniform and attributes to each of the programs
    S.addUniforms();
    testS.addUniforms();
    wreck.addUniforms();
    boundingBox.addUniforms();
    anchor.addUniforms();
    frustumObj.addUniforms();
    frustumObjTest.addUniforms();
}

void saveImage(int width, int height, int nbr)
{
    char buffer[50];

    if (validate)
    {
        sprintf(buffer, "validate%d.png", nbr);
    }
    else
    {
        sprintf(buffer, "image%d.png", nbr);
    }

    GLubyte *data = new GLubyte[4*width*height];
    glReadPixels(0,0,width,height,GL_BGRA,GL_UNSIGNED_BYTE, data);
    cv::Mat imageMat(height, width, CV_8UC4, data);
    cv::flip(imageMat, imageMat, 0);
    cv::imwrite(buffer,imageMat);
}

static void render()
{
    int rotCamIndex = 0;

    if (useCircularPath)
    {
	     CameraParser cp = CameraParser("cameraInfo.txt");
	     cp.getInfo();

	     //rotating Camera
	     float rotCamRadius = cp.camRadius;
	     float rotCamHeight = cp.camHeight;
	     rotCam.updateSpeed(cp.camSpeed);

	     float rotCamX = (sin(glfwGetTime() / rotCam.camSpeed) * rotCamRadius); //glfwGetTime()
	     float rotCamZ = (cos(glfwGetTime() / rotCam.camSpeed) * rotCamRadius);
	     vec3 rotCamPos = vec3(rotCamX, rotCamHeight, rotCamZ);
	     rotCam.updatePosition(rotCamPos);
	     rotCam.updateLookAt(cp.lookAt);

	     Anchor newNode = Anchor();
	     Camera c = Camera(rotCam.eye, rotCam.lookAt, 1);
	     c.updatePerspective(aspect, 45, zNear, 90);
	     c.setRayParameters(1,200);
	     c.createFrustum();
	    // cout << "Weight: " << newNode.weight << "\n";
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
    if (useLawnMowerPath || useCircularPath || useNodePath)
    {
	         if (useLawnMowerPath)
            {
	             V = lawnmower[lawnmowerIndex].cameraView;
	             currentCam = lawnmower[lawnmowerIndex];
	             maxCamIndex = lawnmower.size();
	             lawnmowerIndex++;
	         }
            else if (useCircularPath)
            {
	             V = rotCam.cameraView;
	             currentCam = rotCam;
	             maxCamIndex  = 700;	   		
	         }
            else if (useNodePath)
            {
	             V = path[PATHNUM].camera.cameraView;
	             currentCam = path[PATHNUM].camera;
	             maxCamIndex = path.size() - 1;

				if (PATHNUM < path.size() - 1)
				{
					PATHNUM++;
				}
				else
				{
					PATHNUM = 0;
				}
	         }

	         if (camIndex > maxCamIndex && !useNodePath)
            {
	             exit(0);
	         }

	         camIndex++;
    }
    else
    {
	     currentCam = wasdCam;
	     V = wasdCam.cameraView;
    }

    wreck.draw(M, P, V);
    
    /*
    //TESTING
    testS.draw(M, P, V);

    S.addTransforms(vec3(0, 4, 6), vec3(1, 1, 1), 0, vec3(0, 0, 0));
    S.draw(M, P, V);
    S.addTransforms(vec3(0, 0, 0), vec3(1, 1, 1), 0, vec3(0, 0, 0));
    S.draw(M, P, V);
    frustumObjTest.draw(M, P, V);

    for (int i = 0; i < testAnchor.camera.lookAts.size() - 1; i++) //-1
    { 
  	     S.addTransforms(testAnchor.camera.lookAts[i], vec3(0.5, 0.5, 0.5), 0, vec3(0, 0, 0));
        S.draw(M, P, V);
    }
    */
  	 
    if (drawBoundingBoxes)
    {
	     for (int i = 0; i < sceneObjects.size(); i++)
        {
		      boundingBox.addTransforms(sceneObjects[i].BBtrans,sceneObjects[i].BBscale, 0, vec3(0, 0, 0));
		      boundingBox.draw(M, P, V);
	     }
    }

    if (drawMainBB)
    {
	     boundingBox.addTransforms(wreckBBtrans, wreckBBscale, 0, vec3(0, 0, 0));
	     boundingBox.draw(M, P, V);
    }
   
    if (drawGrid)
    {
	     for (int i = 0; i < grid.size(); i++)
        {
	  	      grid[i].draw(M, P, V);
	     }
    }

    if (drawSDS)
    {
        for (int i = 0; i < validationGrid.size(); i++)
        {
            validationGrid[i].draw(M, P, V);
        }
    }

    if (drawNodes)
    {
        for (int i = 0; i < Nodes.size(); i++)
        {
            Nodes[i].draw(M, P, V);
        }
    }

    if (drawPath)
    {
	     for (int i = 0; i < path.size(); i++)
        {
	  	      path[i].draw(M, P, V);
	     }
    }

    if (drawAnchorPoints)
    {
	     for (int i = 0; i < anchorPts.size(); i++)
        {
	  	      anchorPts[i].draw(M, P, V);
	     }
    }

    //Nodes for those that intersect and AnchorPoints for all 
    if (drawFrustums)
    {
	     Nodes[drawF].draw(M, P, V);
	     Nodes[drawF].fObj.draw(M, P, V);
    }

    if (printLoc)
    {
        cout << "Cam LOC: " << currentCam.eye.x << " " << currentCam.eye.y << " " << currentCam.eye.z << "\n";
        printLoc = false;
    }
	
    if (camIndex % 3 == 0 && camIndex > 0)
    {
   	  saveImage(width, height, imageNum);
   	  imageNum++;
    }    
}


void selectRandomCameras(int sampleNums, int strat){
    if (strat == 10 || strat < 0)
    {
        int grpSize = grid.size() / 10;

        for (int i = 0; i < sampleNums / 10; i++)
        {
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

    for (int i = 0; i < anchorPts.size(); i++)
    {
        anchorPts[i].updateAnchorType(0);
    }
}

void setCameras()
{
    for (int i = 0; i < anchorPts.size(); i++)
    {
	     vec3 lookAtPt;
	     anchorPts[i].createLookAt(anchorPts[i].pos, &lookAtPt);

	     //cout << "origin: " << anchorPts[i].pos.x << " " << anchorPts[i].pos.y << " " << anchorPts[i].pos.z <<"\n";
	     //cout << "lookAt: " << lookAtPt.x << " " << lookAtPt.y << " " << lookAtPt.z <<"\n";
	     Camera c  = Camera(anchorPts[i].pos, lookAtPt, 0);
	     c.updatePerspective(aspect, 45, zNear, zFar);
	     c.setRayParameters(1,200);
	     c.createFrustum();
	     anchorPts[i].setCamera(c, c.startPos.y);
	     frustumObj.setUp(anchorPts[i].camera.frustum);
	     anchorPts[i].fObj = frustumObj;
    }
}

bool boxFrustumIntersectionTest(Frustum fru)
{
    int out;

    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].x > wreckBB.max.x)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].x < wreckBB.min.x)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].y > wreckBB.max.y)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].y < wreckBB.min.y)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].z > wreckBB.max.z)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((fru.Points[i].z < wreckBB.min.z)?1:0); if( out==8 ) return false;

    return true;
}

void checkForBBFrustumInterestion()
{
    Nodes.reserve(anchorPts.size());
    for (int i = 0; i < anchorPts.size(); i++)
    {
	     //0 is flase 
	     bool hit = false;
	     hit = boxFrustumIntersectionTest(anchorPts[i].camera.frustum);

	     if (hit && inBB(anchorPts[i].pos, wreckBB.min, wreckBB.max) == false)
        {
            //fprintf(stderr, "%d\n", i);
		      anchorPts[i].updateAnchorType(hit);
		      Nodes.push_back(anchorPts[i]);
	     }
    }
}

void getAnchorCameraWeights()
{
    float weightTotal = 0;
    float SD = 0;
    float curHighestWeight = 0;

    cout<< "nodes size: " << Nodes.size() << "\n";

    for (int i = 0; i < Nodes.size(); i++)
    {
	    // cout << "Roadmap size: " << roadMap.size() << "\n";
	     Nodes[i].getWeight(wreckBB, hits, roadMap);
	     weightTotal += Nodes[i].weight;
    } 

    nodeWeightMean = weightTotal / Nodes.size();

    for (int i = 0; i < Nodes.size(); i++)
    {
  	     SD += pow(Nodes[i].weight - nodeWeightMean, 2);
    }

    SD = sqrt(SD/Nodes.size());
    highLevelCutOff = SD + nodeWeightMean;

    for (int i = 0; i < Nodes.size(); i++)
    {
	     if (Nodes[i].weight >= highLevelCutOff)
        {
	         highestWeightNodes.push_back(Nodes[i]);
            if (Nodes[i].weight > curHighestWeight)
            {
                curHighestWeight = Nodes[i].weight;
                highestWeightNodeIndex = highestWeightNodes.size() - 1;
            }
	     }
    }
}

float delta(float a, float b)
{
    return abs(a - b);
}

void printHits(Anchor newNode)
{
    if ((newNode.hitting & 16) > 0)
    {
	     cout << "P1: 1\n";
    }
    else
    {
	     cout << "P1: 0\n";
    }

    if ((newNode.hitting & 8) > 0)
    {
	     cout << "P2: 1\n";
    }
    else
    {
	     cout << "P2: 0\n";
    }

    if ((newNode.hitting & 4) > 0) 
    {
	     cout << "P3: 1\n";
    }
    else 
    {
	     cout << "P3: 0\n";
    }

    if ((newNode.hitting & 2) > 0) 
    {
	     cout << "P4: 1\n";
    }
    else
    {
	     cout << "P4: 0\n";
    }

    if ((newNode.hitting & 1) > 0) 
    {
	     cout << "P5: 1\n";
    }
    else 
    {
	     cout << "P5: 0\n";
    }
}

int generateNewNode(int numNodes)
{
    Anchor newNode;
    bool limiter = false;

    if (validate)
    {
        rootNode.pos = validationCell;
        rootNode.camera.lookAt = (rootNode.pos - vec3(wreckBB.max.x, wreckBB.min.y, wreckBB.max.z));
        //This gives the rootnode a parent for going throught the list later. 
        rootNode.root = 1;
        rootNode.pathLength = 0;
        rootNode.parentIndex = -1;
        roadMap.push_back(rootNode);
        Anchor prevNode = rootNode;

        for (int i = 0; i < validationGrid.size(); i++)
        {
            newNode = Anchor(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_anchor_frag.glsl", "white.png", 1, 0, 0);
            newNode.initializeShape();
            newNode.initializeProg();
            newNode.initializeTexture();
            newNode.addUniforms();
            newNode.root = 0;
            newNode.parentIndex = i;
            newNode.pathLength = i + 1;
            newNode.ndex = i + 1;
            newNode.pos = validationCell;
            vec3 newLookAt = validationCell - validationGrid[i].pos;
            newNode.camera.lookAt = newLookAt;
            newNode.createAnchor(pitchIteration, &prevNode, numNodes, i + 1, aspect, zNear, wreckBB, hits, roadMap, realMin, realMax, validate);
            roadMap.push_back(newNode);
            validationGrid[i].weight = newNode.weight;
            prevNode = newNode;
           // cout << "validation weight: " << newNode.weight << endl;
        }

        /*for (int i = 0; i < 11; i++)
        {
            newNode = Anchor(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_anchor_frag.glsl", "white.png", 1, 0, 0);
            newNode.initializeShape();
            newNode.initializeProg();
            newNode.initializeTexture();
            newNode.addUniforms();
            newNode.createAnchor(pitchIteration, &prevNode, numNodes, i + 1, aspect, zNear, wreckBB, hits, roadMap, realMin, realMax, validate);
            newNode.root = 0;
            newNode.parentIndex = i;
            newNode.pathLength = i + 1;
            newNode.ndex = i + 1;
            newNode.pos = validationCell;
            vec3 prevLookAt = roadMap.at(i).camera.lookAt;
            if (prevLookAt.y >= 360)
            {
                prevLookAt.y -= 360;
            }
            vec3 newLookAt = vec3(prevLookAt.x, prevLookAt.y + 30, prevLookAt.z);
            roadMap.push_back(newNode);
            prevNode = newNode;
            cout << "validation weight: " << newNode.weight << endl;
        }
*/
        limiter = true;
    }
    else
    {
        newNode = Anchor(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_anchor_frag.glsl", "white.png", 1, 0, 0);
        Anchor curr;
        newNode.initializeShape();
        newNode.initializeProg();
        newNode.initializeTexture();
        newNode.addUniforms();
        int nodeIndex = -1;
        int currIdx;
        bool high = false;

        ///* For each newNode generated, have every other one expand off of current nodes in the */
        //if(iteration % 2 == 0)
        //{
        //    curr = getNodeFromBinWithLeast();
        //}
        //else
        //{ 
        //    //chose a random node in roadmap until nodes weight is > weightThreshold 
        //    do
        //    {
        //        nodeIndex = rand() % highestWeightNodes.size();
        //        curr = highestWeightNodes.at(nodeIndex);
        //        curr.ndex = roadMap.size() - 1;
        //    } while (curr.weight < highLevelCutOff);
        //}

        curr = getNodeFromBinWithLeast();
        newNode.parentIndex = curr.ndex;
        //creates a random anchor point to expand off of
        //cout << "creating new node\n";
        newNode.createAnchor(pitchIteration, &curr, numNodes, curr.pathLength + 1, aspect, zNear, wreckBB, hits, roadMap, realMin, realMax, validate);
       // cout << "high level: " << highLevelCutOff << "\n";
       // cout << "newNode Weight: " << newNode.weight << "\n";

        newNode.ndex = roadMap.size();
        newNode.anchorType = 1;
        roadMap.push_back(newNode);
        insertIntoSpatialDS(newNode);

        /*if (curr.weight < highWeightAvg && high == true)
        {
            highWeightNodes.erase(highWeightNodes.begin() + currIdx);
            cout << "REMOVED \n";
            highWeightAvg = (((highWeightAvg * (highWeightNodes.size() + 1)) - curr.weight) / highWeightNodes.size());
        }*/

        if (newNode.weight > highLevelCutOff)
        {
            highWeightNodes.push_back(newNode);
            cout << "HWN size : " << highWeightNodes.size() << "\n";
            highWeightAvg = (((highWeightAvg * (highWeightNodes.size() - 1)) + newNode.weight) / highWeightNodes.size());
            cout << "Average: " << highWeightAvg << "\n";
            highLevelCutOff = 0.6 * highWeightAvg;
            cout << "new node added to map: " << roadMap.size() << "\n";
        }
       // cout << "Average: " << highWeightAvg << "\n";

        iteration++;
        pitchIteration++;

        //cout << "completed\n";
        cout << "pathLength: " << newNode.pathLength << "\n";   //WE HAVE A COMPLETE PATH

        printHits(newNode);

        if (limitByPathLength)
        {
            limiter = (newNode.pathLength >= 10);
        }
        else if (limitByTime)
        {

            limiter = (glfwGetTime() >= 400);
        }
        else if (limitByViews)
        {
            limiter = (newNode.hitting >= 31);
        }
    }

	 if (limiter)
    {
        cout<<"DONE\n";

        if (debug)
        {
            double avgRoadmap = 0;

            for (Anchor mapNode : roadMap)
            {
                avgRoadmap += mapNode.weight;
            }

            std::cout << "roadMap size: " << roadMap.size() << std::endl;
            std::cout << "avg roadmap node weight: " << avgRoadmap / roadMap.size() << std::endl;
        }
 
	     // Pull actual path off roadmap from current node up to root node
	     double avgPathWeight = 0;

		  cout << "path size before: " << path.size() << "\n";
        newNode = roadMap.back();

        int p = 0;
        do {
            newNode.addTransforms(newNode.pos, vec3(1, 1, 1), 0, vec3(0, 0, 0));
            path.push_back(newNode);
            //cout<<"path size: " << path.size() << "\n";
            avgPathWeight += newNode.weight;
            //cout << "curr index: " << newNode.ndex << "\n";
            //cout << "parent index: " << newNode.parentIndex << "\n";
            newNode = roadMap[newNode.parentIndex];
            cout << "is it a root? " << newNode.root << "\n";
            cout << "node" << newNode.ndex << " " << newNode.weight << " \n";
            p++;

        } while(newNode.root != 1);

        if (validate)
        {
            float wreckZExtent = wreckBB.max.z - wreckBB.min.z;
            float wreckXExtent = wreckBB.max.x - wreckBB.min.x;
            float wreckArea = wreckZExtent * wreckXExtent;
            float totalArea = (xRealExtent*zRealExtent - wreckArea) / 450.0f;
            unsigned char* imgbuffer = new unsigned char[totalArea * 3.0f];
            cout << "imgbuf size: " << totalArea * 3.0f << endl;
            cout << "valid grid size: " << validationGrid.size() << endl;
            size_t offset = 0;
            for (const Anchor& anchor : validationGrid) {
                imgbuffer[offset * 3] = (unsigned char)(anchor.weight * 255);
                imgbuffer[offset * 3 + 1] = 0;
                imgbuffer[offset * 3 + 2] = 0;
                offset++;
            }

            Image image = Image(imgbuffer, xRealExtent / 15.0, zRealExtent / 15.0, 3, 1);
            image.writeFile("slice.png");
        }

        cout << "path size: " << path.size() << "\n";

        //We built path from the leaf up to root so reverse it
        std::reverse(path.begin(), path.end());

        avgPathWeight /= path.size();
        std::cout << "avg path weight: " << avgPathWeight << std::endl;
   
        if (debug)
        {
            std::cout << "\npercent in high weight list: " << (double)highestWeightNodes.size() / roadMap.size() << std::endl;
        }

        // Write path to file
        std::ofstream outfile(RESOURCE_PATH "path.txt");

        if (outfile.is_open())
        {
            outfile << path.size() << std::endl;
            for (int i = 0; i < path.size(); i++)
            {
                outfile << path[i].pos.x << " " << path[i].pos.y << " " << path[i].pos.z << " " 
                        << path[i].camera.lookAt.x << " " << path[i].camera.lookAt.y  << " " << path[i].camera.lookAt.z << " "
						<< path[i].camera.up.x << " " << path[i].camera.up.y << " " << path[i].camera.up.z << " "
						<< path[i].camera.eye.x << " " << path[i].camera.eye.y << " " << path[i].camera.eye.z << " "
						<< path[i].camera.rightV.x << " " << path[i].camera.rightV.y << " " << path[i].camera.rightV.z << " "
						<< path[i].camera.cameraView[0][0] << " " << path[i].camera.cameraView[0][1] << " " << path[i].camera.cameraView[0][2] << " " << path[i].camera.cameraView[0][3] << " "
						<< path[i].camera.cameraView[1][0] << " " << path[i].camera.cameraView[1][1] << " " << path[i].camera.cameraView[1][2] << " " << path[i].camera.cameraView[1][3] << " "
						<< path[i].camera.cameraView[2][0] << " " << path[i].camera.cameraView[2][1] << " " << path[i].camera.cameraView[2][2] << " " << path[i].camera.cameraView[2][3] << " "
					    << path[i].camera.cameraView[3][0] << " " << path[i].camera.cameraView[3][1] << " " << path[i].camera.cameraView[3][2] << " " << path[i].camera.cameraView[3][3] << " "
						<< path[i].camera.vecU.x << " " << path[i].camera.vecU.y << " " << path[i].camera.vecU.z << " "
						<< path[i].camera.vecV.x << " " << path[i].camera.vecV.y << " " << path[i].camera.vecV.z << " "
						<< path[i].camera.vecW.x << " " << path[i].camera.vecW.y << " " << path[i].camera.vecW.z << " "
						<< path[i].camera.camSpeed
					<< std::endl;
            }

            // Write road map size, average node weight in path, and time to generate path to file
            outfile << roadMap.size() << std::endl;
            outfile << avgPathWeight << std::endl;
        }
        else
        {
            std::cerr << "Unable to open file" << std::endl;
        }

        outfile.close();
        return 1;
    }

    return 0;
}

float getCubicHermite(float flA, float flB, float flC, float flD, float t)
{
	float a = -0.5f * flA + 1.5f * flB - 1.5f * flC + 0.5f * flD;
	float b = flA - 2.5f * flB + 2.0f * flC - 0.5f * flD;
	float c = -0.5f * flA + 0.5f * flC;
	float d = flB;

	return (a * t * t * t) + (b * t * t) + (c * t) + d;
}

Anchor getNodeFromPath(vector<Anchor>& tempPath, int index)
{
	if (index < 0)
	{
		// must be beginning of path
		return tempPath[0];
	}
	else if (index >= tempPath.size())
	{
		// end of path
		return tempPath.back();
	}
	else
	{
		return tempPath[index];
	}
}

void parsePathFile()
{
	std::ifstream infile(RESOURCE_PATH + pathName);
	vector<Anchor> tempPath;
	int pathLength = 0;
	infile >> pathLength;

	for (int i = 0; i < pathLength; ++i)
	{
		Anchor newNode = Anchor(RESOURCE_DIR, "sphere.obj", "tex_vert.glsl", "tex_anchor_frag.glsl", "white.png", 1, 0, 0);
      newNode.initializeShape();
      newNode.initializeProg();
      newNode.initializeTexture();
      newNode.addUniforms();

		newNode.ndex = i;
		newNode.parentIndex = i - 1;
      newNode.anchorType = 1;

		if (i == 0)
		{
			newNode.root = 1;
		}
		else
		{
			newNode.root = 0;
		}

		infile >> newNode.pos.x >> newNode.pos.y >> newNode.pos.z
			>> newNode.camera.lookAt.x >> newNode.camera.lookAt.y >> newNode.camera.lookAt.z
			>> newNode.camera.up.x >> newNode.camera.up.y >> newNode.camera.up.z
			>> newNode.camera.eye.x >> newNode.camera.eye.y >> newNode.camera.eye.z
			>> newNode.camera.rightV.x >> newNode.camera.rightV.y >> newNode.camera.rightV.z
			>> newNode.camera.cameraView[0][0] >> newNode.camera.cameraView[0][1] >> newNode.camera.cameraView[0][2] >> newNode.camera.cameraView[0][3]
			>> newNode.camera.cameraView[1][0] >> newNode.camera.cameraView[1][1] >> newNode.camera.cameraView[1][2] >> newNode.camera.cameraView[1][3]
			>> newNode.camera.cameraView[2][0] >> newNode.camera.cameraView[2][1] >> newNode.camera.cameraView[2][2] >> newNode.camera.cameraView[2][3]
			>> newNode.camera.cameraView[3][0] >> newNode.camera.cameraView[3][1] >> newNode.camera.cameraView[3][2] >> newNode.camera.cameraView[3][3]
			>> newNode.camera.vecU.x >> newNode.camera.vecU.y >> newNode.camera.vecU.z
			>> newNode.camera.vecV.x >> newNode.camera.vecV.y >> newNode.camera.vecV.z
			>> newNode.camera.vecW.x >> newNode.camera.vecW.y >> newNode.camera.vecW.z
			>> newNode.camera.camSpeed;

      newNode.addTransforms(newNode.pos, vec3(1, 1, 1), 0, vec3(0, 0, 0));
		tempPath.push_back(newNode);
	}

	// make a new path with hermite interpolation
	for (int j = 0; j < pathLength * 2 - 1; ++j)
	{
		float interPercent = j / ((float)(pathLength * 2 - 2));
		float x, y, z;
		vec3 u1, v1, w1, u2, v2, w2;
		float tx = interPercent * (pathLength - 1);
		float idx = int(tx);
		float t = tx - floor(tx);

		Anchor A = getNodeFromPath(tempPath, idx - 1);
		Anchor B = getNodeFromPath(tempPath, idx);
		Anchor C = getNodeFromPath(tempPath, idx + 1);
		Anchor D = getNodeFromPath(tempPath, idx + 2);

		x = getCubicHermite(A.pos.x, B.pos.x, C.pos.x, D.pos.x, t);
		y = getCubicHermite(A.pos.y, B.pos.y, C.pos.y, D.pos.y, t);
		z = getCubicHermite(A.pos.z, B.pos.z, C.pos.z, D.pos.z, t);
		vec4 newPos = vec4(x, y, z, 1.0f);

		u1 = B.camera.vecU;
		v1 = B.camera.vecV;
		w1 = B.camera.vecW;
		mat4 cameraUVW1 = mat4(1.0f);
		cameraUVW1[0] = glm::vec4(u1, 1.0f);
		cameraUVW1[1] = glm::vec4(v1, 1.0f);
		cameraUVW1[2] = glm::vec4(w1, 1.0f);

		u2 = C.camera.vecU;
		v2 = C.camera.vecV;
		w2 = C.camera.vecW;
		mat4 cameraUVW2 = mat4(1.0f);
		cameraUVW2[0] = glm::vec4(u2, 1.0f);
		cameraUVW2[1] = glm::vec4(v2, 1.0f);
		cameraUVW2[2] = glm::vec4(w2, 1.0f);

		fquat q1 = quat_cast(cameraUVW1);
		fquat q2 = quat_cast(cameraUVW2);
		fquat slerp = mix(q1, q2, t);
		mat4 newCameraUVW = mat4_cast(slerp);

		mat4 newEye = mat4(1.0f);
		newEye[0][3] = -1.0f * tempPath[idx].camera.eye.x;
		newEye[1][3] = -1.0f * tempPath[idx].camera.eye.y;
		newEye[2][3] = -1.0f * tempPath[idx].camera.eye.z;
		
		vec4 newView = newCameraUVW * newEye * newPos;
	
		Anchor newNode = tempPath[idx];
		newNode.pos = newPos;
		newNode.view = newView;
		path.push_back(newNode);
	}
}

int main(int argc, char **argv)
{
    srand ( time(0) );
    rand();
    g_width = 1200;
    g_height = 800;
    aspect = g_width / g_height;

    

    /* we will always need to load external shaders to set up where */
    if (argc < 3)
    {
        cout << "Wrong format! Try: ./icex resourcesDirectory camera# limiter# [path.txt || validate x y z]" << endl;
        return 1;
    }
    else if (argc == 5)
    {
        pathName = argv[4];
    }
    else if (argc == 8)
    {
        validate = true;
        validationCell = vec3(atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
    }

   RESOURCE_DIR = argv[1] + string("/");
   cameraType = atoi(argv[2]); 
   int limiter = atoi(argv[3]);

   if (cameraType == 1)
   {
   	useLawnMowerPath = true;
   }
   else if (cameraType == 2)
   {
   	useCircularPath = true;
   }
   else if (cameraType == 3)
   {
   	useNodePath = true;
   }
   else
   {
       cout << "Wrong format! Try: ./icex resourcesDirectory camera# limiter# [path.txt]" << endl;
       return 1;
   }

   if (limiter == 1)
   {
       limitByViews= true;
   }
   else if (limiter == 2)
   {
       limitByPathLength = true;
   }
   else if (limiter == 3)
   {
       limitByTime = true;
   }
   else
   {
       cout << "Wrong format! Try: ./icex resourcesDirectory camera# limiter# [path.txt]" << endl;
       return 1;
   }

    // Set error callback as openGL will report errors but they need a call back
    glfwSetErrorCallback(error_callback);

    // Initialize the library.
    if (!glfwInit())
    {
	     return -1;
    }

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // Create a windowed mode window and its OpenGL context.
    window = glfwCreateWindow(g_width, g_height, "textures", NULL, NULL);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!window) 
    {
	     glfwTerminate();
	     return -1;
    }
	
    // Make the window's context current.
    glfwMakeContextCurrent(window);

    if (!gladLoadGL())
    {
	     std::cerr << "Failed to initialize Glad" << std::endl;
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

    // add important wrecks to sceneObj vector
    sceneObjects.push_back(wreck);

    //sceneObjects.push_back(testS); //testing one 

    //make wreckBB
    for (int i = 0; i != sceneObjects.size(); i++)
    {
        wreckBB.addBoundingBox(sceneObjects[i].bb);
    }

	 wreckBB.setPlanes();

    //compute spatial bb
	 //initSpatialDS(wreckBB);
    computeSpatialBB(wreckBB);
    
    cout << "global min: " << wreckBB.min.x << " " << wreckBB.min.y << " " << wreckBB.min.z << "\n"; 
    cout << "global max: " << wreckBB.max.x << " " << wreckBB.max.y << " " << wreckBB.max.z << "\n"; 
    wreckBBscale = vec3((wreckBB.max.x - wreckBB.min.x)/2.0,(wreckBB.max.y - wreckBB.min.y)/2.0,(wreckBB.max.z - wreckBB.min.z)/2.0);	
    wreckBBtrans = vec3(avg(wreckBB.min.x,wreckBB.max.x),avg(wreckBB.min.y,wreckBB.max.y),avg(wreckBB.min.z,wreckBB.max.z));

    createValidationGrid();
    
    //TEST CONTINUED 
    //testAnchor.camera = testCam; 
    //cout << "Roadmap size: " << roadMap.size() << "\n";
    //testAnchor.getWeight(wreckBB, hits, roadMap);
   
    //testAnchor.checkBBPlanes(Ray(vec3(-100,10,0),vec3(1,0,0)),wreckBB,hits, roadMap);

    if (pathName == "")
    {
	     //USING LAWNMOWERPATH
	     if (useLawnMowerPath)
	     {
		      vec3 minBB = wreckBB.min * lawnMowerSizeMin;
		      vec3 maxBB = wreckBB.max * lawnMowerSizeMax;
		      float y = maxBB.y + 1;
		      float xIncrement = (maxBB.x - minBB.x) / 6;

		      for (int i = 0; i <= 6; i += 2)
		      {
		      for (float z = 0; z <= (maxBB.z - minBB.z); z += (maxBB.z - minBB.z) / 700)
		      {
				    Camera c = Camera(vec3(minBB.x + i * xIncrement, y, minBB.z + z), vec3(minBB.x + i * xIncrement + 0.1, 0, minBB.z + z + 0.1), 1); //vec3(wreckBB.min.x+x,y+.5,wreckBB.min.z+z),1);
				    lawnmower.push_back(c);
		      }

		      for (float z = maxBB.z; z >= minBB.z; z -= (maxBB.z - minBB.z) / 700)
		      {
				    Camera c = Camera(vec3(minBB.x + ((i + 1) * xIncrement), y, z), vec3(minBB.x + (i + 1) * xIncrement - 0.1, 0, z - 0.1), 1); //vec3(wreckBB.min.x+x,y+.5,wreckBB.min.z+z),1);
				    lawnmower.push_back(c);
		      }
		      }
	     }

	     // USING NODE PATH 
	     if (useNodePath || cameraType == 0)
	     {
		      vec3 minBB = realMin;
		      vec3 maxBB = realMax;

		      //get sample box information
		      sampleDelta = 10;
		      for (float i = minBB.x; i <= maxBB.x; i += sampleDelta)
		      {
		          for (float j = minBB.y; j <= maxBB.y; j += sampleDelta)
		          {
				        for (float k = minBB.z; k <= maxBB.z; k += sampleDelta)
				        {
					         vec3 tempLoc = vec3(i, j, k);
					         if (inBB(tempLoc, realMin, realMax) == false)
					         {
						          anchor.addPosition(tempLoc);
						          anchor.addTransforms(tempLoc, vec3(1, 1, 1), 0, vec3(0, 0, 0));
						          grid.push_back(anchor);
					         }
				        }
		          }
		      }

		      cout << "grid size: " << grid.size() << "\n";

		      selectRandomCameras(grid.size() / 2, stratifiedNum);
		      setCameras();
		      checkForBBFrustumInterestion();
		      getAnchorCameraWeights();

		      //randomly selects a high weight node to use as root
		      if (highestWeightNodes.size() > 0)
		      {
		          //rootNode = highestWeightNodes[highestWeightNodeIndex];
                rootNode.pos = vec3(wreckBB.max.x, wreckBB.max.y, wreckBB.max.z); //start at upper left corner of bounding box
                rootNode.camera.lookAt = (rootNode.pos - vec3(wreckBB.min.x, wreckBB.min.y, wreckBB.min.z));
		          //This gives the rootnode a parent for going throught the list later. 
		          rootNode.root = 1;
		          rootNode.pathLength = 0;
		          rootNode.parentIndex = -1;

                if (!validate)
                {
                    //sets rootNode in road map
                    rootNode.ndex = 0;
                    roadMap.push_back(rootNode);
                    highWeightNodes.push_back(rootNode);
                    insertIntoSpatialDS(rootNode);
                    highWeightAvg = rootNode.weight;
                }

		          int maxNodes = 20;
		          int currNodes = 1;

		          while (generateNewNode(maxNodes - 1) == 0)
		          {
				        cout << "curNode: " << currNodes << "\n";
				        currNodes++;

				        //for (int i = 0; i < 5; i++)
				        //{
				        //    cout << i<< ": " << hits[i] <<"\n";
				        //}

				        //resetHits();
		          }
		      }
	     }
    }
    else
    {
	     parsePathFile();
    }

    // Loop until the user closes the window.
    while(!glfwWindowShouldClose(window))
    {
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
