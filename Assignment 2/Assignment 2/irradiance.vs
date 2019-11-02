#version 330 core
uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 model;

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTex;

out vec3 normal;
out vec3 position;
out vec2 tc;

void main() {
	gl_Position = projection * modelView * vPosition;
	position = vPosition.xyz;
	normal = mat3(transpose(inverse(model))) * vNormal;
	tc = vTex;
}
