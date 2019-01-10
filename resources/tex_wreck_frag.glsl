#version 330 core

uniform sampler2D fragTexture;

in vec2 vTexCoord;
in vec3 vColor;

out vec4 Outcolor;
in vec3 Lpos;
in vec3 wPos;
in vec3 lightI;
in vec3 veiwer;
in vec3 fragNor;


void main() {
	
	vec3 light = normalize(Lpos-wPos);
	vec3 normal = normalize(fragNor);
	vec3 LI =  normalize(lightI);
	vec3 v = normalize(veiwer-wPos);
	vec3 H = normalize(light+v);
	float spec = pow(max(dot(normal,H),0),2);
	vec3 ambient = .2*vec3( .6353, .8392, .7725);
	vec3 specular= vec3( .0, .0, .0)*spec*LI;
	vec3 diffuse = vec3( .5, .43, .43)*max(dot(light,normal),0)*LI;

//world shader
   vec4 BaseColor = vec4(vColor.x, vColor.y, vColor.z, 1);
   vec4 color = vec4(diffuse+specular+ambient,1);
   vec4 texColor1 = texture(fragTexture, vTexCoord);

    Outcolor = vec4(texColor1.r*color.r+.15, texColor1.g*color.g+.15, texColor1.b*color.b+.15, 1);
			//Outcolor = vec4(texColor1.r, texColor1.g, texColor1.b, 1);

}
