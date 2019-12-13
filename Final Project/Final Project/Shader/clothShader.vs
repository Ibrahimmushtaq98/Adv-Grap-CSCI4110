#version 330 core
uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 model;

in vec4 vPosition;

out vec3 position;

void main() {
	gl_Position = model*projection * modelView * vPosition;
	position = vPosition.xyz;
}
