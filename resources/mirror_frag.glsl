#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;
int valueX;
int valueY;

void main(){
   color = vec4(texture( texBuf, texCoord ).rgb, 1);
   color += vec4(0, 0, 0.05, 0);

}
