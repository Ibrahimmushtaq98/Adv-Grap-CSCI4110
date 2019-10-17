#version 330 core
in vec3 normal;
in vec3 position;
in vec2 tc;
uniform samplerCube tex;
uniform vec4 colour;
uniform vec3 Eye;
uniform vec3 light;
uniform vec4 material;

void main() {
    vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
    float diffuse;
    float specular;

    vec3 tc;
    tc = reflect(-position, normal);
    vec4 colour;

    vec3 L = normalize(light - position);
    vec3 N;
    vec3 R = normalize(reflect(-L,normal));
    N = normalize(normal);

    //diffuse = max(dot(N,L), 0.0);
    diffuse = dot(N,L);
    if(diffuse < 0.0) {
        diffuse = 0.0;
        specular = 0.0;
    } else {
        specular = pow(max(0.0, dot(N,R)),material.w);
    }

    colour = texture(tex, tc);

    gl_FragColor = (material.x*colour + diffuse*colour*material.y + specular*colour *material.z);
    gl_FragColor.a = colour.a;
}

