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

const float airRefract = 1.0003;
const float waterRefract = 1.517;
const float Eta = airRefract/waterRefract;
const float R0 = ((airRefract - waterRefract) * (airRefract - waterRefract)) / ((airRefract + waterRefract) * (airRefract + waterRefract));

void main() {
    vec3 I = normalize(position - camera);
    vec3 N = normalize(normal);
    vec3 R = reflect(-I, N);
    vec3 Rf = refract(-I, N, Eta);
    
    vec4 reflectionColor = texture(tex, normalize(R));
    vec4 refractionColor = texture(tex, normalize(Rf));
    float fresColor = R0 + (1.0 - R0) * pow( (1.0 - dot( -I, N ) ), 10.0);

    //gl_FragColor = vec4(texture(tex, R).rgb, 1.0);
    gl_FragColor = mix(reflectionColor, refractionColor, fresColor);
}