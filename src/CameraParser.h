#include <string>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

struct CameraParser {
public:
	CameraParser(std::string Filename){
		filename = Filename;
	}

	std::string filename;
	float camRadius;
	float camHeight;
	float camSpeed;
	glm::vec3 lookAt;

	void getInfo(){
		ifstream inFile;
		inFile.open("cameraInfo.txt",ios::in);
		if(inFile.fail()){
			cout<<" Could not open " << filename << "\n";
		}
		inFile  >> camRadius;
		inFile >> camHeight;
		inFile >> camSpeed;
		inFile >> lookAt.x;
		inFile >> lookAt.y;
		inFile >> lookAt.z;
		//cout << camRadius << " " << camHeight << " " << camSpeed << " ";
		//cout << lookAt.x << " " << lookAt.y << " " << lookAt.z << "\n";
		inFile.close();
	}

};
