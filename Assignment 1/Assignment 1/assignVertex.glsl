#version 330 core
uniform mat4 modelView;
uniform mat4 projection;

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTex;

out vec3 normal;
out vec3 position;
out vec2 tc;
out vec4 color;

void main() {
	gl_Position = projection * modelView * vPosition;
	position = vPosition.xyz;
	normal = position;

	if(vPosition.y > 14.0){
		color = vec4(1.0,1.0,1.0,1.0); //Snow ontop of mountain
	}else if(vPosition.y > 13.0){
		color = vec4(0.3,0.3,0.3,1.0); //Mountain
	}else if(vPosition.y > 7.0){
		color = vec4(0.0,1.0,0.0,1.0); //Grass
	}else{
		color = vec4(0.0,0.0,1.0,1.0); //Water
	}

	//tc = vTex;
}
