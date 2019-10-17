#version 330 core
uniform mat4 modelView;
uniform mat4 projection;
uniform bool isPlane;

in vec4 vPosition;
in vec3 vNormal;
//in vec2 vTex;

out vec3 normal;
out vec3 position;
//out vec2 tc;

void main() {
	gl_Position = projection * modelView * vPosition;
	position = vPosition.xyz;
	if(isPlane){
		normal = vNormal;
	}else{
		normal = position;
	}
}
