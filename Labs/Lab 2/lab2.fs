#version 330 core

in vec3 normal;
in vec3 fragpos;
//uniform mat4 modelView;

void main() {
    vec3 N = normalize(normal);
    vec3 L = normalize(vec3(1.0,-0.5,0.0)); // X Y Z

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * L;

    //diffuse
    float diff = max(dot(N, L),0.0);
    vec3 diffuse = diff * L;

    //specular
    float specularStrength = 0.5;
    //vec3 viewDir = normalize()

    //output
    vec3 results = (ambient + diffuse) * L;
    gl_FragColor = vec4(results,1.0) * 7.0f; // R G B A

    gl_FragColor.a = 1.0f;

}