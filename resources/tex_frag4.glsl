#version 150 core

uniform sampler2D Texture4;

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
	float spec = pow(max(dot(normal,H),0),5);
	vec3 ambient = .2*vec3( 0.92, 0.98, 0.96);
	vec3 specular= vec3 (0.6, 0.62, 0.68)*spec*LI;
	vec3 diffuse = vec3(0.90, 0.96, 0.9)*max(dot(light,normal),0)*LI;


//world shader
  vec4 BaseColor = vec4(vColor.x, vColor.y, vColor.z, 1);
   vec4 color = vec4(diffuse+specular+ambient,1);
  vec4 texColor1 = texture(Texture4, vTexCoord);

    Outcolor = vec4(texColor1.r*color.r+.05, texColor1.g*color.g+.05, texColor1.b*color.b+.05, 1);
			//Outcolor = vec4(texColor1.r, texColor1.g, texColor1.b, 1);

}
