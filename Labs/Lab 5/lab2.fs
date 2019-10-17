#version 330 core
in vec3 normal;
in vec3 position;
uniform vec4 colour;
uniform vec3 Eye;
uniform vec3 light;
uniform vec4 material;

//void main() {
//    vec3 lightColor = vec3(1.0,0.0,0.0);

//    //Ambients
//    float ambientStrength = material.x;
//    vec3 ambient = ambientStrength * lightColor;

//    //Diffuse
//    vec3 N = normalize(normal);
//    vec3 L = normalize(light - position);
//    float diff = max(dot(N,L), 0.0);
//    vec3 diffuse = diff * lightColor;

//    //Specular
//    float specularStrength = material.z;
//    vec3 V = normalize(Eye - position);
//    vec3 R = normalize(reflect(-L,normal));
//    float spec = pow(max(0.0, dot(N,R)),material.w);
//    vec3 specular = specularStrength * spec * lightColor;

//    vec3 result = vec3(ambient + diffuse + specular) * colour;


//    gl_FragColor = vec4(result, 1.0);
//    //gl_FragColor = vec4(normal, 1.0);

//}


void main() {
    vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
    float diffuse;
    float specular;

    //vec3 tc;
    //tc = reflect(-position, normal);
    //vec4 colour;

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


    gl_FragColor = (material.x*colour + diffuse*colour*material.y + specular*colour *material.z);
    gl_FragColor.a = colour.a;
}



