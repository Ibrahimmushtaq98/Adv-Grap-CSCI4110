#version 330 core
in vec3 normal;
in vec3 position;
in vec2 tc;

uniform samplerCube tex;
uniform vec4 colour;
uniform vec3 Eye;
uniform vec3 light;
uniform vec4 material;

const float airRefract = 1.0003;
const float waterRefract = 1.517;
const float Eta = airRefract/waterRefract;
const float R0 = ((airRefract - waterRefract) * (airRefract - waterRefract)) / ((airRefract + waterRefract) * (airRefract + waterRefract));

void main() {
    vec3 N =normalize(normal);
    vec3 I = normalize(position - Eye);

    vec3 reflectColour = reflect(I,N);
    vec3 refractColour = refract(I,N, Eta);

    vec4 reflectionColor = texture( tex, normalize( reflectColour ) );
    vec4 refractionColor = texture( tex, normalize( refractColour ) );

    float fresColor = R0 + (1.0 - R0) * pow( (1.0 - dot( -I, N ) ), 10.0);
    //vec3 Rt = refract(I,N, 1.33);
    //vec3 Rt = reflect(N,-I);

    //gl_FragColor = vec4(texture(tex,Rt).rgb, 1.0);
    gl_FragColor = mix(reflectionColor,refractionColor,fresColor);
    //gl_FragColor = vec4(reflectColour,1.0);
}

