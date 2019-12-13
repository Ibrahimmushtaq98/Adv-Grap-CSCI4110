#version 330 core
uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 model;

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTextCoord;

out vec3 position;
out vec3 normal;
out vec2 tc;

void main() {
	gl_Position = model*projection * modelView * vPosition;
	//position = vPosition.xyz;
	//normal = vNormal;
	//tc = vTextCoord;
}
