#version 330 core 
in vec3 fragNor;
out vec4 color;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;
in vec3 Lpos;
in vec3 wPos;
in vec3 lightI;
in vec3 veiwer;


void main()
{
	vec3 light = normalize(Lpos-wPos);
	vec3 normal = normalize(fragNor);
	vec3 LI =  normalize(lightI);
	vec3 v = normalize(veiwer-wPos);
	vec3 H = normalize(light+v);
	float spec = pow(max(dot(normal,H),0),shine);
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	//Phong
	ambient = .2*MatAmb;
	diffuse = MatDif*max(dot(light,normal),0)*LI;
	specular = MatSpec*spec*LI;

	//color = vec4(Ncolor, 1.0);
	color = vec4(diffuse + specular + ambient, 1.0);; 

}
	
