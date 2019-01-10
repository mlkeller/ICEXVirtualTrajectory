#version 330 core

uniform sampler2D skyboxTexture;

in vec2 vTexCoord;
in vec3 vColor;

out vec4 Outcolor;

void main() {
//world shader
  vec4 BaseColor = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor1 = texture(skyboxTexture, vTexCoord);

//Outcolor = vec4(texColor1.r*vColor.r+.2, texColor1.g*vColor.g+.2, texColor1.b*vColor.b+.2, 1);
			Outcolor = vec4(texColor1.r, texColor1.g, texColor1.b, 1);
}

