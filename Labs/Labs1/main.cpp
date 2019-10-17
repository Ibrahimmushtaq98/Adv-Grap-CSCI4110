#include <FreeImage.h>
#include <GL/glew.h>
#define GLFW_DLL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <gl/GLU.h>
#include <stdio.h>
#include <stdlib.h>


#define WIDTH 512
#define HEIGHT 512
unsigned char image[HEIGHT][WIDTH][3];
bool flip =true;

void makeImage() {
	int i, j;
	for (j = 0; j < HEIGHT; j++) {
		for (i= 0; i < WIDTH; ++i){
			image[j][i][0] = j % 256;
			image[j][i][1] = 0;
			image[j][i][2] = 0;
		}
	}
}

void display() {
	glLoadIdentity();
	glViewport(0, 0, 511, 511);
	gluOrtho2D(0.0, WIDTH, 0.0, HEIGHT);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
		if (flip) {
			glRasterPos2i(0, 511);
			glPixelZoom(1.0f, -1.0f);
		}
		else {
			glRasterPos2i(0, 0);
			glPixelZoom(1.0, 1.0);
		}
	glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, image);
	glFlush();
}

void loadImage(char* filename) {
	int i, j;
	FIBITMAP* bitmap;
	BYTE* bits;
	bitmap = FreeImage_Load(FIF_PNG, filename, PNG_DEFAULT);
	for (j = 0; j < HEIGHT; j++) {
		bits = FreeImage_GetScanLine(bitmap, j);
		for (i = 0; i < WIDTH; i++) {
			image[j][i][0] = bits[FI_RGBA_RED];
			image[j][i][1] = bits[FI_RGBA_GREEN];
			image[j][i][2] = bits[FI_RGBA_BLUE];
			bits += 3;
		}
	}
	FreeImage_Unload(bitmap);
}

void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
		flip = !flip;
}

void saveImage(char* filename) {
	int i;
	int j;
	FIBITMAP* bitmap;
	BYTE* bits;
	bitmap = FreeImage_Allocate(WIDTH, HEIGHT, 24);
	for (j = 0; j < HEIGHT; j++) {
		bits = FreeImage_GetScanLine(bitmap, j);
		for (i = 0; i < WIDTH; i++) {
			bits[FI_RGBA_RED] = image[j][i][0];
			bits[FI_RGBA_GREEN] = image[j][i][1];
			bits[FI_RGBA_BLUE] = image[j][i][2];
			bits += 3;
		}
	}
	FreeImage_Save(FIF_PNG, bitmap, filename, PNG_DEFAULT);
}
int main(int argc, char** argv) {
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		fprintf(stderr, "can't initialize GLFW\n");
	}
	makeImage();
	saveImage((char*)"image.png");
	loadImage((char*)"image.png");
	window = glfwCreateWindow(512, 512, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwSetKeyCallback(window, key_callback);
		glfwMakeContextCurrent(window);
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error starting GLEW: %s\n", glewGetErrorString(error));
		exit(0);
	}
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window)) {
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
}