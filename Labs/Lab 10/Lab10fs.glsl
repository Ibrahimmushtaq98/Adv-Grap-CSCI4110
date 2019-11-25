#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(std140, binding=1) uniform bufferVals {
	vec4 lightPos;
} myBufferVals;

layout(std140, binding=1) uniform buff21 {
	vec4 colour;
} buff2;

layout (location = 0) in vec3 Normal;
layout(location = 0) out vec4 outColor;

void main() {
	vec3 N = normalize(Normal);
	vec3 L = normalize(vec3(buff2.colour));

	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * L;

	float diff = max(dot(N, L), 0.0);

	vec3 diffuse = diff * L;
	float specularStrength = 0.5;

	vec3 results = (ambient + diffuse) * L;

	outColor = vec4(results, 1.0) * 7.0f;
}