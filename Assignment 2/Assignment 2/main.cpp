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

#include "Shaders.h"
#include "ShadersV2.h"
#include "texture.h"
#include "CustomCameraKeyboard.h"
#include "tiny_obj_loader.h"
#include "CallBack.hpp"

#define _USE_MATH_DEFINES

const std::string TITLE_NAME = "Base Project";
const unsigned int SHADOW_WIDTH = 1280, SHADOW_HEIGHT = 720;

//Call backs for GLFW
void framebufferSizeCallback(GLFWwindow* window, int w, int h);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void keyCallback(GLFWwindow* window);
void error_callback(int error, const char* description);

//Custom Functiom
void display();
void drawGUI();
void renderSphere();

//Shader Program
Shaders* mainShaderProgram;
Shaders* cubemapShaderProgram;

struct _stat buf;
int fid;
float eyex, eyey, eyez;	// current user position

GLuint program;

//Sphere Buffer
GLuint objVAO;			// vertex object identifier
int triangles;			// number of triangles
int triangles2;
GLuint ibuffer;			// index buffer identifier

//Plane Buffer
GLuint planeVAO;
GLuint planeBuffer;

GLuint shadowBuffer;
GLuint shadowTex;
GLuint colourTex;

GLuint vbuffer[2];
GLenum buffs[] = { GL_COLOR_ATTACHMENT0 };

//ImGUI Stuff
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
ImVec4 light = ImVec4(0.0, 0.0, 0.3, 1.0f);
ImVec4 material = ImVec4(0.3, 0.7, 0.7, 150.0);
ImVec4 colour = ImVec4(1.0, 0.0, 0.0, 1.0);
ImVec4 eye = ImVec4(0.0, 0.0, 0.0, 1.0);

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
		fid = _open("sphere.bin", _O_WRONLY | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
		result = _write(fid, &nv, (sizeof vertices));
		result = _write(fid, &nn, (sizeof normals));
		result = _write(fid, &ni, (sizeof indices));
		result = _write(fid, vertices, nv * (sizeof GLfloat));
		result = _write(fid, normals, nn * (sizeof GLfloat));
		result = _write(fid, indices, ni * (sizeof GLuint));
		_close(fid);
	}

	/*
	*  load the vertex coordinate data
	*/
	glGenBuffers(1, &vbuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer[0]);
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
	//glGenFramebuffers(1, &shadowBuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);

	//glGenTextures(1, &shadowTex);
	//glBindTexture(GL_TEXTURE_2D, shadowTex);
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, SHADOW_WIDTH, SHADOW_HEIGHT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);

	//glGenTextures(1, &colourTex);
	//glBindTexture(GL_TEXTURE_2D, colourTex);
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, SHADOW_WIDTH, SHADOW_HEIGHT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colourTex, 0);

	//glBindTexture(GL_TEXTURE_2D, 0);

	mainShaderProgram->useShader();
	vPosition = glGetAttribLocation(mainShaderProgram->getShaderID(), "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(mainShaderProgram->getShaderID(), "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)(nv * sizeof(vertices)));
	glEnableVertexAttribArray(vNormal);

}

void initCubemap() {

}


void display(void) {
	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 view = camera.GetViewMatrix();
	renderSphere();
	glFinish();
}

int main(int argc, char** argv) {
	GLFWwindow* window;
	const char* glsl_version = "#version 330";

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

	//if (glDebugMessageCallback != NULL) {
	glDebugMessageCallback((GLDEBUGPROC)openGlDebugCallback, NULL);
	//}
	glEnable(GL_DEBUG_OUTPUT);

	std::cout << "GLEW Version:   " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "GPU Vendor:     " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "GPU Model:      " << glGetString(GL_RENDERER) << std::endl << std::endl;

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

	mainShaderProgram = new Shaders("shader.vs", "shader.fs");
	mainShaderProgram->dumpProgram((char*)"Main Shader Program for Assignment 2");

	cubemapShaderProgram = new Shaders("cube.vs", "cube.fs");
	cubemapShaderProgram->dumpProgram((char*)"Cube Map Shader Program");

	initSphere();
	initCubemap();

	eyex = 0.0;
	eyez = 0.0;
	eyey = 7.0;
	light = ImVec4(eyex, eyey, eyez, 1.0f);

	glfwSwapInterval(1);
	lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		//Frame stuff
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		glfwPollEvents();
		//drawGUI();

		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		display();
		keyCallback(window);

		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}


	//ImGui_ImplOpenGL3_Shutdown();
	//ImGui_ImplGlfw_Shutdown();
	//ImGui::DestroyContext();

	glfwTerminate();

}
void drawGUI() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	{
		ImGui::Begin("Shader Controls");
		ImGui::ColorEdit4("clear color", (float*)&clear_color);
		ImGui::SliderFloat4("light", (float*)&light, 0.0, 1.0);
		ImGui::SliderFloat4("material", (float*)&material, 0.0, 100.0);
		ImGui::SliderFloat4("colour", (float*)&colour, 0.0, 1.0);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
}
void renderSphere() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	mainShaderProgram->useShader();

	glm::mat4 view = camera.GetViewMatrix();

	//view = glm::lookAt(glm::vec3(eyex, eyey, eyez),
	//	glm::vec3(0.0f, 0.0f, 0.0f),
	//	glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 scale = glm::mat4(glm::vec4(0.5, 0.0, 0.0, 0.0),
		glm::vec4(0.0, 0.5, 0.0, 0.0),
		glm::vec4(0.0, 0.0, 0.5, 0.0),
		glm::vec4(0.5, 0.5, 0.5, 1.0));
	//glm::mat4 proj = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 100.0f);
	projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

	glm::mat4 shadowMatrix = scale * projection * view;

	mainShaderProgram->setMat4("modelView", view);
	mainShaderProgram->setMat4("projection", projection);
	mainShaderProgram->setMat4("shadowMatrix", shadowMatrix);
	mainShaderProgram->setVec4("colour", colour.x, colour.y, colour.z, colour.w);
	mainShaderProgram->setVec4("material", material.x, material.y, material.z, material.w);
	mainShaderProgram->setVec3("light", light.x, light.y, light.z);
	mainShaderProgram->setBool("isPlane", false);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, shadowTex);
	//glViewport(0, 0, WIDTH, HEIGHT);

	//glViewport(0, 0, SHADOW_WIDTH - 1, SHADOW_HEIGHT - 1);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(20.0f, 6.0f);
	//glDrawBuffers(1, buffs);

	//int status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	//if (status != GL_FRAMEBUFFER_COMPLETE) {
	//	printf("bad framebuffer object\n");
	//}
	//glBindVertexArray(objVAO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	//glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, NULL);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDisable(GL_POLYGON_OFFSET_FILL);

	glBindVertexArray(objVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glDrawElements(GL_TRIANGLES, 3 * triangles, GL_UNSIGNED_INT, NULL);
}