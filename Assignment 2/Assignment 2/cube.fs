#version 330 core
in vec3 normal;
in vec3 position;
in vec2 tc;

uniform samplerCube tex;
//uniform vec4 colour;
//uniform vec3 Eye;
//uniform vec3 light;
//uniform vec4 material;

void main() {
    vec3 tc;
    tc = reflect(position, normal);
    gl_FragColor = texture(tex,tc);
}