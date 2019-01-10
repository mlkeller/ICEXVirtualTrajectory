#version  330 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec3 vColor;
out vec2 vTexCoord;
out vec3 fragNor;
out vec3 Lpos;
out vec3 wPos;
out vec3 lightI;
out vec3 veiwer;

void main() {

  veiwer = vec3(vertPos);
  Lpos = vec3(1,2,2);
	wPos = vec3(V * M* vertPos);
	lightI = vec3(1,1,1);

  vec3 lightDir = vec3(1, 1, 1);
  vec4 vPosition;

  /* First model transforms */
  gl_Position = P * V* M * vec4(vertPos.xyz, 1.0);

  fragNor = (V* M * vec4(vertNor, 0.0)).xyz;
  /* a color that could be blended - or be shading */
  vColor = vec3(max(dot(fragNor, normalize(lightDir)), 0));
  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;
}
