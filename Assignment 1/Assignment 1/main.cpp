/************************************************************
*                   CSCI 4110 Assignment 1
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
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <io.h>
#include <string>
#include <sstream> 

#include "Shaders.h"
#include "ShadersV2.h"
#include "texture.h"
#include "CustomCameraKeyboard.h"
#include "DiamondSquare.h"

#define _USE_MATH_DEFINES


const int WIDTH = 960;
const int HEIGHT = 600;
const std::string TITLE_NAME = "Assignment 1";

//Call backs for GLFW
void framebufferSizeCallback(GLFWwindow* window, int w, int h);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void keyCallback(GLFWwindow* window);
void error_callback(int error, const char* description);

//Custom Functiom
void display(DiamondSquare ds);
void createGeometry(DiamondSquare ds);

CameraKeyBoard camera(glm::vec3(0.0f, 15.0f, 3.0f));
float lastX = (float)WIDTH / 2.0f;
float lastY = (float)HEIGHT / 2.0f;
bool firstM = true;

//Frames Stuff
int frameCount = 0;
double lastTime = 0.0;
float deltaTime = 0.0f;

GLuint programId;
GLuint vertexBuffer;
GLuint indexBuffer;
GLenum positionBufferId;

GLuint program;

struct _stat buf;
int fid;
float eyex, eyey, eyez;	// current user position

glm::mat4 projection;	// projection matrix

GLuint objVAO;			// vertex object identifier
int triangles;			// number of triangles
GLuint ibuffer;			// index buffer identifier

GLuint tBuffer;
Shaders *shaderProgram;

int main(int argc, char** argv) {
	GLFWwindow* window;
	int vs, fs;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		fprintf(stderr, "can't initialize GLFW\n");
		glfwTerminate();
		return -1;
	}

	window = glfwCreateWindow(WIDTH, HEIGHT, TITLE_NAME.c_str(), NULL, NULL);
	if (!window){
		fprintf(stderr, "Failed to Create Window\n");
		glfwTerminate();
		return -2;
	}

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(window);

	GLenum error = glewInit();
	if (error != GLEW_OK) {
		printf("Error starting GLEW: %s\n", glewGetErrorString(error));
		return -3;
	}

	std::cout << "GLEW Version:   " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "GPU Vendor:     " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "GPU Model:      " << glGetString(GL_RENDERER) << std::endl <<std::endl;

	//Setting up global OpenGL suff
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glViewport(0, 0, WIDTH, HEIGHT);

	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

	//Shaders Program Init
	shaderProgram = new Shaders("assignVertex.glsl", "assignFrag.glsl");
	shaderProgram->dumpProgram((char*)"Assignment 1 Shaders Program");

	std::ifstream settingFiles;
	settingFiles.open("setting.txt");
	std::string str;
	std::string settingTexts[3];
	int i = 0;
	while (std::getline(settingFiles, str)) {
		if (i == 3) {
			break;
		}
		settingTexts[i++] = str;
	}
	settingFiles.close();

	//Geometry Init
	DiamondSquare ds(stoi(settingTexts[0]), stoi(settingTexts[1]), stoi(settingTexts[2]));
	ds.GenerateField();
	createGeometry(ds);


	eyex = 0.0;
	eyez = 0.0;
	eyey = 3.0;
	//camera.Position = glm::vec3(eyex, eyey, eyez);

	glfwSwapInterval(1);
	lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {

		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		//showFPS(window);
		keyCallback(window);

		display(ds);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;

}

void createGeometry(DiamondSquare ds) {

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ds.sizeVertexPos, ds.vertexPositionData, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)* ds.sizeIndex,ds.indexData, GL_STATIC_DRAW);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void framebufferSizeCallback(GLFWwindow* window, int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).

	if (h == 0)
		h = 1;

	float ratio = 1.0f * w / h;

	glfwMakeContextCurrent(window);

	glViewport(0, 0, w, h);

	projection = glm::perspective(0.7f, ratio, 0.1f, 100.0f);

}

void display(DiamondSquare ds) {
	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	int viewLoc;
	int projLoc;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderProgram->useShader();

	shaderProgram->setMat4("modelView", view);
	shaderProgram->setMat4("projection", projection);
	shaderProgram->setVec3("light", eyex, eyey, eyez);
	shaderProgram->setVec3("Eye", eyex, eyey, eyez);
	shaderProgram->setVec4("material", 0.05, 0.8, 0.1, 0.03);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(positionBufferId, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void*)0);
	glEnableVertexAttribArray(positionBufferId);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLE_STRIP, ds.sizeVertexPos, GL_UNSIGNED_SHORT, (void*)0);
	glDisableVertexAttribArray(positionBufferId);

	glFinish();
	
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos){
	if (firstM)
	{
		lastX = xpos;
		lastY = ypos;
		firstM = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void keyCallback(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboardEntry(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboardEntry(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboardEntry(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboardEntry(RIGHT, deltaTime);
}