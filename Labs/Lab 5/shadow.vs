#version 330 core
uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 shadowMatrix;
uniform bool isPlane;
in vec4 vPosition;
in vec3 vNormal;
out vec3 normal;
out vec3 position;
out vec4 sPosition;
void main() {
	gl_Position = projection * modelView * vPosition;
	position = vPosition.xyz;
	normal = vNormal;
	sPosition = shadowMatrix * vPosition;
}