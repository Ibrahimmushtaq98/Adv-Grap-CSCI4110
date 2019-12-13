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
#include "Cloth.h"

#define _USE_MATH_DEFINES
#define DEBUG

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
void drawGUI();

float eyex, eyey, eyez;	// current user position

//Shaders Stuff
glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);
glm::vec4 light = glm::vec4(0.0, 0.0, 0.3, 1.0f);
glm::vec4 material = glm::vec4(0.3, 0.7, 0.7, 150.0);
glm::vec4 colour = glm::vec4(1.0, 1.0, 0.0, 1.0);
glm::vec4 eye = glm::vec4(0.0, 0.0, 0.0, 1.0);
glm::vec4 translation = glm::vec4(glm::vec3(2.0f, 1.0f, 2.0f), 1.0);

bool start = false;
bool trueMove = false;

GLFWwindow* window;

int main(int argc, char** argv) {
	srand(time(NULL));
	const char* glsl_version = "#version 330";

	//Context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_FORWARD_COMPAT);

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

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, WIDTH, HEIGHT);

	//Setting up ImGUI
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//ImGui::StyleColorsDark();
	//ImGui_ImplGlfw_InitForOpenGL(window, true);
	//ImGui_ImplOpenGL3_Init(glsl_version);

	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

	Cloth mesh(10, 10, 2, 0.01);

	eyex = 0.0;
	eyez = 0.0;
	eyey = 3.0;
	light = glm::vec4(eyex, eyey, eyez, 1.0f);

	glfwSwapInterval(1);
	lastTime = glfwGetTime();

	glm::vec3 eye(5.0, 10.0, -3.0);
	glm::vec3 look(5.0, 0.0, 5.0);
	glm::vec3 up(0.0, 1.0, 0.0);

	glm::mat4 view = glm::lookAt(eye, look, up);

	while (!glfwWindowShouldClose(window)) {
		//Frame stuff
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		glfwPollEvents();

		//drawGUI();

		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		view = camera.GetViewMatrix();
		mesh.renderGeometry(projection, view);
		if (start) {
			//mesh.updateGeometry(deltaTime);
		}
		keyCallback(window);

		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext();
	glfwTerminate();

}

//Draw the objects
void display(void) {
	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFinish();
}

//Draws the GUI
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
		ImGui::BulletText("K Key is for reseting the sims");
		ImGui::Unindent();
		ImGui::Text("");

		ImGui::Checkbox("Start/Pause Sim", (bool*)&start);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
}