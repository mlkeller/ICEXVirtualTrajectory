#version  330 core
layout(location = 0) in vec3 vertPos;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
out vec2 texCoord;

void main()
{
    gl_Position =P*V*M *vec4(vertPos, 1);
	texCoord = (vertPos.xy+vec2(1, 1))/2.0;
}
