/************************************************************
*                   CSCI 4110 Lab 3
*
*  Basic OpenGL program that shows how to set up a
*  VAO and some basic shaders.  This program draws
*  a cube or sphere depending upon whether CUBE or
*  SPHERE is defined.
*
**********************************************************/
#define GLM_FORCE_RADIANS
#define _CRT_SECURE_NO_WARNINGS

#include <GL/glew.h>
#define GLFW_DLL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <stdio.h>
#include "Shaders.h"
#include "tiny_obj_loader.h"
#include "texture.h"
//#include <FreeImage.h>
#include <iostream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <Windows.h>
#define _USE_MATH_DEFINES
#include <math.h>

//#define INTERLEAVED

//#define CUBE
#define SPHERE
struct _stat buf;
int fid;
float eyex, eyey, eyez;	// current user position

double theta, phi;		// user's position  on a sphere centered on the object
double r;				// radius of the sphere

GLuint program;

glm::mat4 projection;	// projection matrix

GLuint objVAO;			// vertex object identifier
int triangles;			// number of triangles
GLuint ibuffer;			// index buffer identifier

GLuint tBuffer;
//unsigned int texture;

double getSeconds() {
	LARGE_INTEGER freq, val;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&val);
	return (double)val.QuadPart / (double)freq.QuadPart;
}

#ifdef CUBE
/*
 * This version of the init procedure produces the
 * data for drawing a cube.  The vertex and index
 * data are stored in constant arrays which are copied
 * into the buffers.  This code should be familiar
 * from class.
 */
void init() {
	GLuint vbuffer;
	GLint vPosition;
	GLint vNormal;

	glGenVertexArrays(1, &objVAO);
	glBindVertexArray(objVAO);

	GLfloat vertices[8][4] = {
			{ -1.0, -1.0, -1.0, 1.0 },		//0
			{ -1.0, -1.0, 1.0, 1.0 },		//1
			{ -1.0, 1.0, -1.0, 1.0 },		//2
			{ -1.0, 1.0, 1.0, 1.0 },		//3
			{ 1.0, -1.0, -1.0, 1.0 },		//4
			{ 1.0, -1.0, 1.0, 1.0 },		//5
			{ 1.0, 1.0, -1.0, 1.0 },		//6
			{ 1.0, 1.0, 1.0, 1.0 }			//7
	};

	GLfloat normals[8][3] = {
			{ -1.0, -1.0, -1.0 },			//0
			{ -1.0, -1.0, 1.0 },			//1
			{ -1.0, 1.0, -1.0 },			//2
			{ -1.0, 1.0, 1.0 },				//3
			{ 1.0, -1.0, -1.0 },			//4
			{ 1.0, -1.0, 1.0 },				//5
			{ 1.0, 1.0, -1.0 },				//6
			{ 1.0, 1.0, 1.0 }				//7
	};

	GLuint indexes[36] = { 0, 1, 3, 0, 2, 3,
		0, 4, 5, 0, 1, 5,
		2, 6, 7, 2, 3, 7,
		0, 4, 6, 0, 2, 6,
		1, 5, 7, 1, 3, 7,
		4, 5, 7, 4, 6, 7 };

	triangles = 12;

	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);

	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

	glUseProgram(program);
	vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*) sizeof(vertices));
	glEnableVertexAttribArray(vNormal);

}
#endif

#ifdef SPHERE
void init() {
	GLuint vbuffer;
	GLint vPosition;
	GLint vNormal;
	GLfloat* vertices;
	GLfloat* normals;
	GLuint* indices;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	int nv;
	int nn;
	int ni;
	int i;

	glGenVertexArrays(1, &objVAO);
	glBindVertexArray(objVAO);

	int result = _stat("sphere.bin", &buf);

	if (result == 0) {
		fid = _open("sphere.bin", _O_RDONLY | _O_BINARY);
		result = _read(fid, &nv, (sizeof vertices));
		result = _read(fid, &nn, (sizeof normals));
		result = _read(fid, &ni, (sizeof indices));
		triangles = ni / 3;
		printf("v: %d, n: %d i: %d\n", nv, nn, ni);
		vertices = new GLfloat[nv];
		result = _read(fid, vertices, nv * (sizeof GLfloat));
		normals = new GLfloat[nn];
		result = _read(fid, normals, nn * (sizeof GLfloat));
		indices = new GLuint[ni];
		result = _read(fid, indices, ni * sizeof(GLuint));
		_close(fid);

	}
	else {
		/*  Load the obj file */

		std::string err = tinyobj::LoadObj(shapes, materials, "sphere.obj", 0);

		if (!err.empty()) {
			std::cerr << err << std::endl;
			return;
		}

		/*  Retrieve the vertex coordinate data */

		nv = (int)shapes[0].mesh.positions.size();
		vertices = new GLfloat[nv];
		for (i = 0; i < nv; i++) {
			vertices[i] = shapes[0].mesh.positions[i];
		}

		/*  Retrieve the vertex normals */

		nn = (int)shapes[0].mesh.normals.size();
		normals = new GLfloat[nn];
		for (i = 0; i < nn; i++) {
			normals[i] = shapes[0].mesh.normals[i];
		}

		/*  Retrieve the triangle indices */

		ni = (int)shapes[0].mesh.indices.size();
		triangles = ni / 3;
		indices = new GLuint[ni];
		for (i = 0; i < ni; i++) {
			indices[i] = shapes[0].mesh.indices[i];
		}
		fid = _open("buddha.bin", _O_WRONLY | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
		result = _write(fid, &nv, (sizeof vertices));
		result = _write(fid, &nn, (sizeof normals));
		result = _write(fid, &ni, (sizeof indices));
		result = _write(fid, vertices, nv * (sizeof GLfloat));
		result = _write(fid, normals, nn * (sizeof GLfloat));
		result = _write(fid, indices, ni * (sizeof GLuint));
		_close(fid);
	}
#ifdef INTERLEAVED
	GLfloat* combined = new GLfloat[nv + nn];
	int k = 0;
	for (i = 0; i < nv / 3; i++) {
		combined[k++] = vertices[3 * i];
		combined[k++] = vertices[3 * i + 1];
		combined[k++] = vertices[3 * i + 2];
		combined[k++] = normals[3 * i];
		combined[k++] = normals[3 * i + 1];
		combined[k++] = normals[3 * i + 2];
	}
	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, (nv + nn) * sizeof(GLfloat), combined, GL_STATIC_DRAW);
	glUseProgram(program);
	vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 6 * (sizeof GLfloat), 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 6 * (sizeof GLfloat),
		(void*)(combined + 3 * (sizeof GLfloat)));
	glEnableVertexAttribArray(vNormal);
#endif

	unsigned int verts = nv / 3;
	unsigned int nt = 2 * verts;
	//int x = 0, y = 0, z = 0;
	GLfloat* tex = new GLfloat[nt];
	for (i = 0; i < verts; i++) {
		GLfloat x = vertices[3 * i];
		GLfloat y = vertices[3 * i + 1];
		GLfloat z = vertices[3 * i + 2];
		theta = atan2(x, z);
		phi = atan2(y, sqrt(x * x + z * z));
		//tex[2 * i] = (theta + M_PI) / (2 * M_PI);
		tex[2 * i] = fabs(theta) / M_PI;
		tex[2 * i + 1] = phi / M_PI;
	}

	/*
	*  load the vertex coordinate data
	*/
	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, (nv + nn + nt) * sizeof(GLfloat), NULL,GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, nv * sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, nv * sizeof(GLfloat), nn * sizeof(GLfloat), normals);
	glBufferSubData(GL_ARRAY_BUFFER, (nv + nn) * sizeof(GLfloat), nt * sizeof(GLfloat),tex);
	/*
	*  load the vertex indexes
	*/
	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni * sizeof(GLuint), indices, GL_STATIC_DRAW);

	unsigned int vTex = glGetAttribLocation(program, "vTex");
	glVertexAttribPointer(vTex, 2, GL_FLOAT, GL_FALSE, 0, (void*)((nv +nn) * sizeof(GLfloat)));
	glEnableVertexAttribArray(vTex);
	/*
	*  link the vertex coordinates to the vPosition
	*  variable in the vertex program.  Do the same
	*  for the normal vectors.
	*/
	glUseProgram(program);
	vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)(nv * sizeof(vertices)));
	glEnableVertexAttribArray(vNormal);


	//Texture
/*
	Texture* texture = loadTexture("checkerboard.jpg");
	glGenTextures(1, &tBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tBuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture->width, texture->height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGB, GL_UNSIGNED_BYTE, texture->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
*/

	//Cube
	Cube* texture = loadCube("./VancouverConventionCentre");
	glGenTextures(1, &tBuffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tBuffer);
	for (i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
			0, GL_RGBA, texture -> width, texture->height,
			0, GL_RGB, GL_UNSIGNED_BYTE, texture->data[i]);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
#endif

void display(void) {
	glm::mat4 view;
	int viewLoc;
	int projLoc;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);

	view = glm::lookAt(glm::vec3(eyex, eyey, eyez),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 5.0f));

	viewLoc = glGetUniformLocation(program, "modelView");
	glUniformMatrix4fv(viewLoc, 1, 0, glm::value_ptr(view));
	projLoc = glGetUniformLocation(program, "projection");
	glUniformMatrix4fv(projLoc, 1, 0, glm::value_ptr(projection));

	GLint lightLoc = glGetUniformLocation(program, "light");
	glUniform3f(lightLoc, eyex, eyey, eyez);

	GLint eyeLoc = glGetUniformLocation(program, "Eye");
	glUniform3f(eyeLoc, eyex, eyey, eyez);

	GLint materialLoc = glGetUniformLocation(program, "material");
	glUniform4f(materialLoc, 0.05, 0.8, 0.1, 0.03);

	glBindVertexArray(objVAO);
	double t1 = getSeconds();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, NULL);
	glFinish();
	//printf("%f\n", getSeconds() - t1);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		phi -= 0.1;
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		phi += 0.1;
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		theta += 0.1;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		theta -= 0.1;

	eyex = (float)(r * sin(theta) * cos(phi));
	eyey = (float)(r * sin(theta) * sin(phi));
	eyez = (float)(r * cos(theta));

}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}


int main(int argc, char** argv) {
	int fs;
	int vs;
	GLFWwindow* window;

	// start by setting error callback in case something goes wrong

	glfwSetErrorCallback(error_callback);

	// initialize glfw

	if (!glfwInit()) {
		fprintf(stderr, "can't initialize GLFW\n");
	}

	// create the window used by our application

	window = glfwCreateWindow(512, 512, "Laboratory Four", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// establish framebuffer size change and input callbacks

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwSetKeyCallback(window, key_callback);

	// now initialize glew our extension handler

	glfwMakeContextCurrent(window);

	GLenum error = glewInit();
	if (error != GLEW_OK) {
		printf("Error starting GLEW: %s\n", glewGetErrorString(error));
		exit(0);
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.48, 0.48, 0.48, 1.0);
	glViewport(0, 0, 512, 512);

	projection = glm::perspective(0.7f, 1.0f, 1.0f, 100.0f);

	vs = buildShader(GL_VERTEX_SHADER, (char*)"lab2.vs");
	fs = buildShader(GL_FRAGMENT_SHADER, (char*)"lab2.fs");
	program = buildProgram(vs, fs, 0);
	dumpProgram(program, (char*)"Lab 4 shader program");
	init();

	eyex = 0.0;
	eyez = 0.0;
	eyey = 10.0;

	theta = 1.5;
	phi = 1.5;
	r = 10.0;

	glfwSwapInterval(1);

	// GLFW main loop, display model, swapbuffer and check for input

	while (!glfwWindowShouldClose(window)) {
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

}
