#version 330 core
in vec3 normal;
in vec3 position;
in vec2 tc;

uniform samplerCube tex;
uniform vec4 colour;
uniform vec3 Eye;
uniform vec3 light;
uniform vec4 material;
uniform vec3 camera;
uniform float radius;
uniform float Theta;

const float airRefract = 1.0003;
const float waterRefract = 1.555;
const float Eta = airRefract/waterRefract;
//https://en.wikipedia.org/wiki/Schlick%27s_approximation 
const float R0 = ((airRefract - waterRefract) * (airRefract - waterRefract)) / ((airRefract + waterRefract) * (airRefract + waterRefract));

const float PI = 3.14159265359;

void main() {
    vec3 I = normalize(position - camera);
    vec3 N = normalize(normal);

    //Part 3
    //V = Lcos(theta)U + Lsin(theta)W
    //Where L = Radius
    //      theta = uniformly distributed angles
    //      U,W = orthogonal vectors to the normal

    float L = radius;
    float theta = Theta;
    vec3 newNormal = vec3(normal.x, normal.z, normal.y);
    vec3 U = (cross(newNormal, normal));
    vec3 W = (cross(U, normal));

    vec3 V = ((L * cos(theta)* U) + (L*sin(theta)*W));
    vec3 NV = normalize(normal + V);


    //Part 1
    vec3 R = reflect(-I, V);
    vec3 Rf = refract(-I, V, Eta);
    
    vec4 reflectionColor = texture(tex, normalize(R));
    vec4 refractionColor = texture(tex, normalize(Rf));

    //https://en.wikipedia.org/wiki/Fresnel_equations
    float fresColor = R0 + (1.0 - R0) * pow( (1.0 - dot( -I, V ) ), 10.0);
    gl_FragColor = mix(reflectionColor, refractionColor, fresColor);

    //gl_FragColor = vec4(V, 1.0);

}