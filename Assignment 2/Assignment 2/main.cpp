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
#include <vector>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include <string>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Shaders.h"
#include "ShadersV2.h"
#include "texture.h"
#include "CustomCameraKeyboard.h"
#include "tiny_obj_loader.h"
#include "CallBack.hpp"

#define _USE_MATH_DEFINES
//#define DEBUG

const std::string TITLE_NAME = "Assignment 2";
const unsigned int SHADOW_WIDTH = 1280, SHADOW_HEIGHT = 720;

//Call backs for GLFW
void framebufferSizeCallback(GLFWwindow* window, int w, int h);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void keyCallback(GLFWwindow* window);
void error_callback(int error, const char* description);

//Custom Functiom
void display();
void initSphere();
void initCubemap();
void renderSphere();
void renderCubeMap();
void drawGUI();

//Shader Program
Shaders* mainShaderProgram;
Shaders* cubemapShaderProgram;

double theta, phi;		// user's position  on a sphere centered on the object
double r;				// radius of the sphere

struct _stat buf;
int fid;
float eyex, eyey, eyez;	// current user position

GLuint program;

//Sphere Buffer
GLuint objVAO;			// vertex object identifier
int triangles;			// number of triangles
int triangles2;
GLuint ibuffer;			// index buffer identifier
GLuint tBuffer;
GLuint tBuffer2;

//Plane Buffer
GLuint cubemapVAO;
GLuint cubemapBuffer;

GLuint shadowBuffer;
GLuint shadowTex;
GLuint colourTex;

GLuint vbuffer[2];

//Shaders Stuff
glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);
glm::vec4 light = glm::vec4(0.0, 0.0, 0.3, 1.0f);
glm::vec4 material = glm::vec4(0.3, 0.7, 0.7, 150.0);
glm::vec4 colour = glm::vec4(1.0, 0.0, 0.0, 1.0);
glm::vec4 eye = glm::vec4(0.0, 0.0, 0.0, 1.0);

GLfloat Theta = M_PI;
GLfloat radius = 0.2;

static int e = 0;

int main(int argc, char** argv) {
	GLFWwindow* window;
	const char* glsl_version = "#version 330";

	//Context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		fprintf(stderr, "can't initialize GLFW\n");
		glfwTerminate();
		return -1;
	}

	window = glfwCreateWindow(WIDTH, HEIGHT, TITLE_NAME.c_str(), NULL, NULL);
	if (!window) {
		fprintf(stderr, "Failed to Create Window\n");
		glfwTerminate();
		return -2;
	}

	//Setting Callbacks 
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(window);

	GLenum error = glewInit();
	if (error != GLEW_OK) {
		printf("Error starting GLEW: %s\n", glewGetErrorString(error));
		return -3;
	}
#ifdef DEBUG
	//Debug Message
	if (glDebugMessageCallback != NULL) {
		glDebugMessageCallback((GLDEBUGPROC)openGlDebugCallback, NULL);
	}
	glEnable(GL_DEBUG_OUTPUT);
#endif // DEBUG

	std::cout << "GLEW Version:   " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "GPU Vendor:     " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "GPU Model:      " << glGetString(GL_RENDERER) << std::endl << std::endl;

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, WIDTH, HEIGHT);

	//Setting up ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

	//Initializing Shaders
	mainShaderProgram = new Shaders("irradiance.vs", "irradiance.fs");
	mainShaderProgram->dumpProgram((char*)"Main Shader Program for Assignment 2");

	cubemapShaderProgram = new Shaders("cube.vs", "cube.fs");
	cubemapShaderProgram->dumpProgram((char*)"Cube Map Shader Program");

	//Initializing Geometry
	initSphere();
	initCubemap();

	eyex = 0.0;
	eyez = 0.0;
	eyey = 3.0;
	light = glm::vec4(eyex, eyey, eyez, 1.0f);

	glfwSwapInterval(1);
	lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		//Frame stuff
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		glfwPollEvents();

		drawGUI();

		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		display();
		keyCallback(window);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

}

void initSphere() {
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
	glGenBuffers(1, &vbuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, (nv + nn + nt) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, nv * sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, nv * sizeof(GLfloat), nn * sizeof(GLfloat), normals);

	/*
	*  load the vertex indexes
	*/
	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni * sizeof(GLuint), indices, GL_STATIC_DRAW);

	/*
	*  link the vertex coordinates to the vPosition
	*  variable in the vertex program.  Do the same
	*  for the normal vectors.
	*/
	mainShaderProgram->useShader();
	vPosition = glGetAttribLocation(mainShaderProgram->getShaderID(), "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(mainShaderProgram->getShaderID(), "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)((nv/2) * sizeof(vertices)));
	glEnableVertexAttribArray(vNormal);


	Cube* texture = loadCube("./BlurMap");
	glGenTextures(1, &tBuffer2);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tBuffer2);
	for (i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGBA, texture->width, texture->height,
			0, GL_RGB, GL_UNSIGNED_BYTE, texture->data[i]);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}
void initCubemap() {
	GLint vPosition;
	GLint vNormal;

	glGenVertexArrays(1, &cubemapVAO);
	glBindVertexArray(cubemapVAO);

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

	triangles2 = 12;

	glGenBuffers(1, &vbuffer[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);

	glGenBuffers(1, &cubemapBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubemapBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

	cubemapShaderProgram->useShader();
	vPosition = glGetAttribLocation(cubemapShaderProgram->getShaderID(), "vPosition");
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(cubemapShaderProgram->getShaderID(), "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*) sizeof(vertices));
	glEnableVertexAttribArray(vNormal);

	Cube* texture = loadCube("./CubeMaps");
	glGenTextures(1, &tBuffer);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tBuffer);
	int i;
	for (i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGBA, texture->width, texture->height,
			0, GL_RGB, GL_UNSIGNED_BYTE, texture->data[i]);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}
void display(void) {
	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderCubeMap();
	renderSphere();
	glFinish();
}
void renderSphere() {
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	mainShaderProgram->useShader();

	glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);



	mainShaderProgram->setMat4("modelView", view);
	mainShaderProgram->setMat4("projection", projection);
	mainShaderProgram->setVec4("colour", colour.x, colour.y, colour.z, colour.w);
	mainShaderProgram->setVec4("material", material.x, material.y, material.z, material.w);
	mainShaderProgram->setVec3("light", light.x, light.y, light.z);
	mainShaderProgram->setMat4("model", model);
	mainShaderProgram->setVec3("camera", camera.Position.x, camera.Position.y, camera.Position.z);
	mainShaderProgram->setFloat("radius", radius);
	mainShaderProgram->setFloat("Theta", Theta);

	glBindVertexArray(objVAO);
	glActiveTexture(GL_TEXTURE0);
	//Show Original Cube Map
	if (e == 0) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, tBuffer);
	}
	//Show Blur Cube Map
	else if (e == 1) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, tBuffer2);
	}
	//Show Uniform Sampling part
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, tBuffer2);
	}
	mainShaderProgram->setInt("e", e);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, NULL);
}
void renderCubeMap() {
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);

	cubemapShaderProgram->useShader();
	glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	//view = camera.GetViewMatrix();
	cubemapShaderProgram->setMat4("modelView", view);
	cubemapShaderProgram->setMat4("projection", projection);

	glBindVertexArray(cubemapVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubemapBuffer);
	glDrawElements(GL_TRIANGLES, 3 * triangles2, GL_UNSIGNED_INT, NULL);

}
void drawGUI() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	{
		ImGui::Begin("Shader Controls");
		ImGui::BulletText("Camera Control Instruction");
		ImGui::Indent();
		ImGui::BulletText("You can move your mouse like a FPS");
		ImGui::BulletText("WASD Key is for Traversing the scene");
		ImGui::BulletText("H Key is for disabling/reinable the mouse");
		ImGui::Unindent();
		ImGui::Text("");

		ImGui::Text("Select Which Parts do you want to see");
		ImGui::RadioButton("Part 0 (Reflection and Refraction)", &e, 0);
		ImGui::RadioButton("Part 1 (Diffuse Reflection)", &e, 1);
		ImGui::RadioButton("Part 2 (Diffuse Reflection)", &e, 2);

		if (e == 2) {
			ImGui::SliderFloat("theta", (float*)&Theta, -2.0 * M_PI, 2.0 * M_PI);
			ImGui::SliderFloat("radius", (float*)&radius, 0.0, 10.0);
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
}