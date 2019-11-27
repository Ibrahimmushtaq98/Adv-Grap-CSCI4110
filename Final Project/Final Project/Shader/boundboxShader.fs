#version 330 core
in vec3 normal;
in vec3 position;

uniform vec4 colour;

void main() {
    gl_FragColor = colour;
}