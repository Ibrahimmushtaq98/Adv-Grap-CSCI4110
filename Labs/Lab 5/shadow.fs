#version 420 core
in vec3 normal;
in vec3 position;
in vec4 sPosition;
uniform vec4 colour;
uniform vec3 light;
uniform vec4 material;
uniform sampler2DShadow tex;
void main() {
    vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
    float diffuse;
    vec3 L = normalize(light);
    vec3 N;
    vec3 R = normalize(reflect(-(light-position),normal));
    float specular;
    float shadow;
    N = normalize(normal);
    diffuse = dot(N,L);
    if(diffuse < 0.0) {
        diffuse = 0.0;
        specular = 0.0;
    } else {
        specular = pow(max(0.0, dot(N,R)),material.w);
    }
    shadow = textureProj(tex, sPosition);
    gl_FragColor = material.x*colour;
    //gl_FragColor = vec4(shadow);
    gl_FragColor += min((material.y*diffuse*colour + material.z*white*specular)*shadow,vec4(1.0));
    gl_FragColor.a = colour.a;
}