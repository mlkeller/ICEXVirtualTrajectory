#version 330 core

uniform sampler2D fragTexture;
uniform float anchorType;

in vec2 vTexCoord;
in vec3 vColor;

out vec4 Outcolor;
in vec3 Lpos;
in vec3 wPos;
in vec3 lightI;
in vec3 veiwer;
in vec3 fragNor;


void main() {
	
	vec3 ambient;
	vec3 specular;
	vec3 diffuse;
	vec3 light = normalize(Lpos-wPos);
	vec3 normal = normalize(fragNor);
	vec3 LI =  normalize(lightI);
	vec3 v = normalize(veiwer-wPos);
	vec3 H = normalize(light+v);
	float spec = pow(max(dot(normal,H),0),10);

	if(anchorType < 0){
       ambient = vec3( 0.9, .9, .9);
       specular= 5*vec3(0.9922, 0.941176, 0.90784)*spec*LI;
	   diffuse = vec3(.9,.9,.9)*max(dot(light,normal),0)*LI;
	}else if(anchorType == 0 ){
	    ambient = vec3( 0.5, .8, .3);
        specular= 5*vec3(0.5, 0.8, 0.3)*spec*LI;
	    diffuse = vec3(.8,.5,.3)*max(dot(light,normal),0)*LI;
	}else{
		ambient = vec3( 0.6, .2, .8);
        specular= 5*vec3(0.5, 0.1, 0.7)*spec*LI;
	    diffuse = vec3(.6,.2,.8)*max(dot(light,normal),0)*LI;
	}

	 //world shader
	vec4 BaseColor = vec4(vColor.x, vColor.y, vColor.z, 1);
    vec4 color = vec4(diffuse+specular+ambient,1);
    vec4 texColor1 = texture(fragTexture, vTexCoord);

    Outcolor = vec4(texColor1.r*color.r, texColor1.g*color.g, texColor1.b*color.b,1);

    //Outcolor = vec4(texColor1.r, texColor1.g, texColor1.b,1);

}

