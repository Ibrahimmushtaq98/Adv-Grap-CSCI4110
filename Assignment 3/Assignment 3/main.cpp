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
#include <string>
#include <cstdlib>
#include <ctime>

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
#include "tribes.h"
#include "BoundingBox.h"

#define _USE_MATH_DEFINES
//#define DEBUG

const std::string TITLE_NAME = "Assignment 3";
const int SIZE = 4;
bool show = true;

//Call backs for GLFW
void framebufferSizeCallback(GLFWwindow* window, int w, int h);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void keyCallback(GLFWwindow* window);
void error_callback(int error, const char* description);

//Custom Functiom
void display();
void initModel(char* filelocation,Shaders* shaderProgram,GLuint& objectVAO,GLuint& vbuffer,GLuint& ibuffer,int& triangle);
void initCube(Shaders* shaderProgram, GLuint& objectVAO, GLuint& vbuffer, GLuint& ibuffer, int& triangle);
float randFloat(int min, int max);
bool checkCollisionArray(BoundBox b1, std::vector<BoundBox> b2);
void renderGround();
void renderMonkey();
void renderFlags();
void renderBox();
void randomMonkey();
void drawGUI();
void FixedUpdate();

//Shader Program
Shaders* monkeyProgram;
Shaders* groundProgram;
Shaders* flagProgram;
Shaders* boundboxProgram;

BoundingBox* bb;
BoundingBox* bbMonkey;
std::vector<BoundBox> redMonkey;
std::vector<BoundBox> blueMonkey;

float eyex, eyey, eyez;	// current user position

//Buffers
int triangles[SIZE];
GLuint ibuffer[SIZE];
GLuint objVAO[SIZE];
GLuint vbuffer[SIZE];

//Shaders Stuff
glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);
glm::vec4 light = glm::vec4(0.0, 0.0, 0.3, 1.0f);
glm::vec4 material = glm::vec4(0.3, 0.7, 0.7, 150.0);
glm::vec4 colour = glm::vec4(1.0, 1.0, 0.0, 1.0);
glm::vec4 eye = glm::vec4(0.0, 0.0, 0.0, 1.0);
glm::vec4 translation = glm::vec4(glm::vec3(2.0f, 1.0f, 2.0f), 1.0);

bool showBox = true;

Tribes RedTribes[5];
Tribes BlueTribes[5];

int main(int argc, char** argv) {
	srand(time(NULL));
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
	{
		std::cout << "GLEW Version:   " << glewGetString(GLEW_VERSION) << std::endl;
		std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
		std::cout << "GLSL Version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
		std::cout << "GPU Vendor:     " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "GPU Model:      " << glGetString(GL_RENDERER) << std::endl << std::endl;
	}

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
	monkeyProgram = new Shaders("Shader/monkeyShader.vs", "Shader/monkeyShader.fs");
	monkeyProgram->dumpProgram((char*)"Main Shader Program for Assignment 3");

	groundProgram = new Shaders("Shader/groundShader.vs", "Shader/groundShader.fs");
	groundProgram->dumpProgram((char*)"Ground Shader Program for Assignment 3");

	flagProgram = new Shaders("Shader/monkeyShader.vs", "Shader/monkeyShader.fs");
	boundboxProgram = new Shaders("Shader/boundboxShader.vs", "Shader/boundboxShader.fs");

	//Initializing Geometry
	initModel((char*)"Model/ground.obj", groundProgram, objVAO[0], vbuffer[0], ibuffer[0], triangles[0]);
	initModel((char*)"Model/monkey.obj", monkeyProgram, objVAO[1], vbuffer[1], ibuffer[1], triangles[1]);
	initModel((char*)"Model/flags.obj", flagProgram, objVAO[2], vbuffer[2], ibuffer[2], triangles[2]);
	initCube(boundboxProgram, objVAO[3], vbuffer[3], ibuffer[3], triangles[3]);
	bb = new BoundingBox((char*)"Model/ground.obj", true);
	bbMonkey = new BoundingBox((char*)"Model/monkey.obj", false);

	randomMonkey();

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
		if (k_press) {
			redMonkey.clear();
			blueMonkey.clear();
			randomMonkey();
		}
		FixedUpdate();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

}

void initModel(char* filelocation, Shaders* shaderProgram, GLuint& objectVAO, GLuint &vbuffer, GLuint &ibuffer,int &triangle) {

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

	glGenVertexArrays(1, &objectVAO);
	glBindVertexArray(objectVAO);

	/*  Load the obj file */

	std::string err = tinyobj::LoadObj(shapes, materials, filelocation, 0);

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
	triangle = ni / 3;
	indices = new GLuint[ni];
	for (i = 0; i < ni; i++) {
		indices[i] = shapes[0].mesh.indices[i];
	}

	/*
	*  load the vertex coordinate data
	*/
	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, (nv + nn) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
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
	shaderProgram->useShader();
	vPosition = glGetAttribLocation(shaderProgram->getShaderID(), "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(shaderProgram->getShaderID(), "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)((nv / 2) * sizeof(vertices)));
	glEnableVertexAttribArray(vNormal);
}
void initCube(Shaders* shaderProgram, GLuint& objectVAO, GLuint& vbuffer, GLuint& ibuffer, int& triangle) {
	GLint vPosition;
	GLint vNormal;

	glGenVertexArrays(1, &objectVAO);
	glBindVertexArray(objectVAO);

	GLfloat vertices[8][4] = {
			{ -0.5, -0.5, -0.5, 1.0 },		//0
			{ -0.5, -0.5, 0.5, 1.0 },		//1
			{ -0.5, 0.5, -0.5, 1.0 },		//2
			{ -0.5, 0.5, 0.5, 1.0 },		//3
			{ 0.5, -0.5, -0.5, 1.0 },		//4
			{ 0.5, -0.5, 0.5, 1.0 },		//5
			{ 0.5, 0.5, -0.5, 1.0 },		//6
			{ 0.5, 0.5, 0.5, 1.0 }			//7
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

	triangle = 12;

	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);

	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

	shaderProgram->useShader();
	vPosition = glGetAttribLocation(shaderProgram->getShaderID(), "vPosition");
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(shaderProgram->getShaderID(), "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*) sizeof(vertices));
	glEnableVertexAttribArray(vNormal);

}
void display(void) {
	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderGround();
	renderBox();
	renderMonkey();
	renderFlags();
	glFinish();
}
void renderGround() {
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	groundProgram->useShader();
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);

	groundProgram->setMat4("modelView", view);
	groundProgram->setMat4("projection", projection);
	groundProgram->setVec4("colour", colour.x, colour.y, colour.z, colour.w);
	groundProgram->setVec4("material", material.x, material.y, material.z, material.w);
	groundProgram->setVec3("light", light.x, light.y, light.z);

	glBindVertexArray(objVAO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer[0]);
	glDrawElements(GL_TRIANGLES, 3 * triangles[0], GL_UNSIGNED_INT, NULL);
}


void renderMonkey() {
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	monkeyProgram->useShader();
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(translation.x, translation.y, translation.z));

	monkeyProgram->setMat4("modelView", view);
	monkeyProgram->setMat4("projection", projection);
	monkeyProgram->setVec4("material", material.x, material.y, material.z, material.w);
	monkeyProgram->setVec3("light", light.x, light.y, light.z);

	for (int i = 0; i < 5; i++) {
		monkeyProgram->useShader();
		monkeyProgram->setVec4("colour", RedTribes[i].color.x, RedTribes[i].color.y, RedTribes[i].color.z, RedTribes[i].color.w);
		monkeyProgram->setMat4("model", RedTribes[i].position);

		glBindVertexArray(objVAO[1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer[1]);
		glDrawElements(GL_TRIANGLES, 3 * triangles[1], GL_UNSIGNED_INT, NULL);

		if (!showBox) {
			continue;
		}

		glm::vec3 size = glm::vec3(RedTribes[i].max.x - RedTribes[i].min.x, RedTribes[i].max.y - RedTribes[i].min.y, RedTribes[i].max.z - RedTribes[i].min.z);
		glm::vec3 center = glm::vec3((RedTribes[i].min.x + RedTribes[i].max.x) / 2, (RedTribes[i].min.y + RedTribes[i].max.y) / 2, (RedTribes[i].min.z + RedTribes[i].max.z) / 2);
		glm::mat4 transform = glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);

		transform[3][0] = RedTribes[i].position[3][0];
		transform[3][1] = RedTribes[i].position[3][1];
		transform[3][2] = RedTribes[i].position[3][2];
		transform[3][3] = RedTribes[i].position[3][3];

		boundboxProgram->useShader();
		boundboxProgram->setMat4("model", transform);
		boundboxProgram->setVec4("colour", RedTribes[i].color.x, RedTribes[i].color.y, RedTribes[i].color.z, RedTribes[i].color.w);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(objVAO[3]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer[3]);
		glDrawElements(GL_TRIANGLES, 3 * triangles[3], GL_UNSIGNED_INT, NULL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	}

	for (int i = 0; i < 5; i++) {
		monkeyProgram->useShader();
		monkeyProgram->setVec4("colour", BlueTribes[i].color.x, BlueTribes[i].color.y, BlueTribes[i].color.z, BlueTribes[i].color.w);
		monkeyProgram->setMat4("model", BlueTribes[i].position);

		glBindVertexArray(objVAO[1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer[1]);
		glDrawElements(GL_TRIANGLES, 3 * triangles[1], GL_UNSIGNED_INT, NULL);

		if (!showBox) {
			continue;
		}

		glm::vec3 size = glm::vec3(BlueTribes[i].max.x - BlueTribes[i].min.x, BlueTribes[i].max.y - BlueTribes[i].min.y, BlueTribes[i].max.z - BlueTribes[i].min.z);
		glm::vec3 center = glm::vec3((BlueTribes[i].min.x + BlueTribes[i].max.x) / 2, (BlueTribes[i].min.y + BlueTribes[i].max.y) / 2, (BlueTribes[i].min.z + BlueTribes[i].max.z) / 2);
		glm::mat4 transform = glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);

		transform[3][0] = BlueTribes[i].position[3][0];
		transform[3][1] = BlueTribes[i].position[3][1];
		transform[3][2] = BlueTribes[i].position[3][2];
		transform[3][3] = BlueTribes[i].position[3][3];

		boundboxProgram->useShader();
		boundboxProgram->setMat4("model", transform);
		boundboxProgram->setVec4("colour", BlueTribes[i].color.x, BlueTribes[i].color.y, BlueTribes[i].color.z, BlueTribes[i].color.w);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(objVAO[3]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer[3]);
		glDrawElements(GL_TRIANGLES, 3 * triangles[3], GL_UNSIGNED_INT, NULL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
void renderFlags() {
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	flagProgram->useShader();
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	model = glm::translate(model, glm::vec3(25.0f, 0.0f, -25.0f));

	flagProgram->setMat4("modelView", view);
	flagProgram->setMat4("projection", projection);
	flagProgram->setVec4("material", material.x, material.y, material.z, material.w);
	flagProgram->setVec3("light", light.x, light.y, light.z);

	//Blue FLags
	flagProgram->setVec4("colour", 0.0f, 0.0, 1.0, 1.0f);
	flagProgram->setMat4("model", model);
	glBindVertexArray(objVAO[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer[2]);
	glDrawElements(GL_TRIANGLES, 3 * triangles[2], GL_UNSIGNED_INT, NULL);

	//Red Flgas
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
	model = glm::translate(model, glm::vec3(-25.0f, 0.0f, 25.0f));
	flagProgram->setVec4("colour", 1.0f, 0.0, 0.0, 1.0f);
	flagProgram->setMat4("model", model);
	glBindVertexArray(objVAO[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer[2]);
	glDrawElements(GL_TRIANGLES, 3 * triangles[2], GL_UNSIGNED_INT, NULL);
}
void renderBox() {
	if (!showBox) {
		return;
	}

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	boundboxProgram->useShader();
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(translation.x, translation.y, translation.z));

	boundboxProgram->setMat4("modelView", view);
	boundboxProgram->setMat4("projection", projection);
	boundboxProgram->setVec4("colour", 1.0f, 1.0f, 1.0f, 1.0f);

	for (int i = 0; i < bb->bb.size(); i++) {
		glm::vec3 size = glm::vec3(bb->bb[i].max.x - bb->bb[i].min.x, bb->bb[i].max.y - bb->bb[i].min.y, bb->bb[i].max.z - bb->bb[i].min.z);
		glm::vec3 center = glm::vec3((bb->bb[i].min.x + bb->bb[i].max.x) / 2, (bb->bb[i].min.y + bb->bb[i].max.y) / 2, (bb->bb[i].min.z + bb->bb[i].max.z) / 2);
		glm::mat4 transform = glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);
		boundboxProgram->useShader();
		boundboxProgram->setMat4("model", transform);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(objVAO[3]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer[3]);
		glDrawElements(GL_TRIANGLES, 3 * triangles[3], GL_UNSIGNED_INT, NULL);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

		ImGui::Checkbox("Show Box", (bool*)&showBox);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
}
void randomMonkey() {
	glm::vec3 Redgoals = glm::vec3(-14.0f, 0.0f, 14.0f);
	glm::vec3 Bluegoals = glm::vec3(14.0f, 0.0f, -14.0f);
	int i;
	//Red Monkey
	for (i = 0; i < 5; i++) {
		glm::mat4 model = glm::mat4(1.0f);

		RedTribes[i].tribeClan = "Red";
		RedTribes[i].color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		RedTribes[i].acceleration = randFloat(1.0, 2.0f);
		RedTribes[i].speed = randFloat(1.0, 2.0f);
		float x = randFloat(5, 15), z = randFloat(-15, -5);
		RedTribes[i].position = glm::translate(model, glm::vec3(x, 1.0f, z));
		RedTribes[i].min = glm::vec3(bbMonkey->bb[0].min.x + x, 0, bbMonkey->bb[0].min.z + z);
		RedTribes[i].max = glm::vec3(bbMonkey->bb[0].max.x + x, bbMonkey->bb[0].max.y + 1, bbMonkey->bb[0].max.z + z);

		BoundBox tmp;
		tmp.min = RedTribes[i].min;
		tmp.max = RedTribes[i].max;

		while (checkCollisionArray(tmp, redMonkey) == true || checkCollisionArray(tmp, bb[0].bb) == true) {
			x = randFloat(5, 15), z = randFloat(-15, -5);
			RedTribes[i].position = glm::translate(model, glm::vec3(x, 1.0f, z));
			RedTribes[i].min = glm::vec3(bbMonkey->bb[0].min.x + x, 0, bbMonkey->bb[0].min.z + z);
			RedTribes[i].max = glm::vec3(bbMonkey->bb[0].max.x + x, bbMonkey->bb[0].max.y + 1, bbMonkey->bb[0].max.z + z);
			tmp.min = RedTribes[i].min;
			tmp.max = RedTribes[i].max;
		}
		redMonkey.push_back(tmp);

		glm::vec3 curPos = glm::vec3(RedTribes[i].position[3][0], RedTribes[i].position[3][1], RedTribes[i].position[3][2]);
		RedTribes[i].velocity = glm::normalize(Redgoals - curPos);
	}

	//Blue Monkey
	for (i = 0; i < 5; i++) {
		glm::mat4 model = glm::mat4(1.0f);

		float x = randFloat(-15, -5); 
		float z = randFloat(5, 15);

		//model = glm::translate(model, glm::vec3(x, 0.0, z));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f,1,0.0f));
		model = glm::translate(model, glm::vec3(-x, 1.0f, -z));

		BlueTribes[i].tribeClan = "Blue";
		BlueTribes[i].color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		BlueTribes[i].speed = randFloat(1.0f, 2.0f);
		BlueTribes[i].acceleration = randFloat(1.0, 2.0f);
		BlueTribes[i].position = model;
		BlueTribes[i].min = glm::vec3(bbMonkey->bb[0].min.x + x, 0, bbMonkey->bb[0].min.z + z);
		BlueTribes[i].max = glm::vec3(bbMonkey->bb[0].max.x + x, bbMonkey->bb[0].max.y + 1, bbMonkey->bb[0].max.z + z);

		BoundBox tmp;
		tmp.min = BlueTribes[i].min;
		tmp.max = BlueTribes[i].max;

		while (checkCollisionArray(tmp, blueMonkey) == true || checkCollisionArray(tmp, bb[0].bb) == true) {
			float x = randFloat(-15, -5);
			float z = randFloat(5, 15);
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1, 0.0f));
			model = glm::translate(model, glm::vec3(-x, 1.0f, -z));
			BlueTribes[i].position = model;
			BlueTribes[i].min = glm::vec3(bbMonkey->bb[0].min.x + x, 0, bbMonkey->bb[0].min.z + z);
			BlueTribes[i].max = glm::vec3(bbMonkey->bb[0].max.x + x, bbMonkey->bb[0].max.y + 1, bbMonkey->bb[0].max.z + z);
			tmp.min = BlueTribes[i].min;
			tmp.max = BlueTribes[i].max;
		}
		blueMonkey.push_back(tmp);
		glm::vec3 curPos = glm::vec3(BlueTribes[i].position[3][0], BlueTribes[i].position[3][1], BlueTribes[i].position[3][2]);
		RedTribes[i].velocity = glm::normalize(Bluegoals - curPos);
	}

}
float randFloat(int min, int max) {
	return std::rand() % (max + 1 - min) + min;
}
void FixedUpdate() {
	glm::vec3 Bluegoals = glm::vec3(14.0f, 0.0f, -14.0f);
	glm::vec3 Redgoals = glm::vec3(-14.0f, 0.0f, 14.0f);


	for (int i = 0; i < 5; i++) {
		glm::vec3 curPos = glm::vec3(BlueTribes[i].position[3][0], BlueTribes[i].position[3][1], BlueTribes[i].position[3][2]);
		glm::vec3 directs = glm::normalize(Bluegoals - curPos);
		glm::mat4 model = BlueTribes[i].position;
		directs.y = 0;

		if (glm::distance(Bluegoals, curPos) > 5.0f) {
			model = glm::translate(model, directs * (deltaTime * -1 * BlueTribes[i].speed));
			BlueTribes[i].position = model;
			BlueTribes[i].min = glm::vec3(bbMonkey->bb[0].min.x + model[3][0], 0, bbMonkey->bb[0].min.z + model[3][2]);
			BlueTribes[i].max = glm::vec3(bbMonkey->bb[0].max.x + model[3][0], bbMonkey->bb[0].max.y + 1, bbMonkey->bb[0].max.z + model[3][2]);

			BoundBox tmp;
			tmp.min = BlueTribes[i].min;
			tmp.max = BlueTribes[i].max;
			blueMonkey.erase(blueMonkey.begin() + i);
			

			while(checkCollisionArray(tmp, bb[0].bb) == true || checkCollisionArray(tmp, blueMonkey) == true) {
				model = glm::translate(model, glm::vec3(1.0f * deltaTime * BlueTribes[i].speed, 0.0f, 0.0f));
				BlueTribes[i].min = glm::vec3(bbMonkey->bb[0].min.x + model[3][0], 0, bbMonkey->bb[0].min.z + model[3][2]);
				BlueTribes[i].max = glm::vec3(bbMonkey->bb[0].max.x + model[3][0], bbMonkey->bb[0].max.y + 1, bbMonkey->bb[0].max.z + model[3][2]);
				tmp.min = BlueTribes[i].min;
				tmp.max = BlueTribes[i].max;
			}
			BlueTribes[i].position = model;
			blueMonkey.insert(blueMonkey.begin() + i, tmp);

		}

	}

	for (int i = 0; i < 5; i++) {
		glm::vec3 curPos = glm::vec3(RedTribes[i].position[3][0], RedTribes[i].position[3][1], RedTribes[i].position[3][2]);
		glm::vec3 directs = glm::normalize(Redgoals - curPos);
		glm::mat4 model = RedTribes[i].position;
		directs.y = 0;

		if (glm::distance(Redgoals, curPos) > 5.0f) {
			model = glm::translate(model, directs * (deltaTime * RedTribes[i].speed));
			RedTribes[i].position = model;
		}

	}


	//float angles = atan2f(Bluegoals.x - curPos.x, Bluegoals.z - curPos.z);
	//
	////std::cout << glm::distance(Bluegoals, curPos) << std::endl;

	//	if (l_press) {
	//		model = glm::rotate(model, -glm::radians(angles), glm::vec3(0.0, 1.0, 0.0));
	//	}
	//}
}

bool checkCollisionArray(BoundBox b1, std::vector<BoundBox> b2) {

	for (int i = 0; i < b2.size(); i++) {
		if ((b1.min.x <= b2[i].max.x && b1.max.x >= b2[i].min.x) &&
			(b1.min.y <= b2[i].max.y && b1.max.y >= b2[i].min.y) &&
			(b1.min.z <= b2[i].max.z && b1.max.z >= b2[i].min.z)) {
			return true;
		}
	}

	return false;
}