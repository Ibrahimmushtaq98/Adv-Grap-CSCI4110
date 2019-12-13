#pragma once

#ifndef CLOTH_SPRING_V2
#define CLOTH_SPRING_V2

#include <GL/glew.h>
#include <vector>
#include <set>
#include <algorithm>

#include "ShadersV2.h"
#include "ClothParticle.h" 

class ClothSpring {
public:
	ClothSpring();

	void setSpringConstant(float springConstant);
	float getSpringConstant();

	void setDampeningConstant(float dampeningConstant);
	float getDampeningConstant();

	void setEndParticles(std::pair<ClothParticle*, ClothParticle*> endParticles);
	void setEndParticles(ClothParticle* endA, ClothParticle* endB);
	std::pair<ClothParticle*, ClothParticle*> getEndParticles();

	void setRestLength(float restLength);
	float getRestLength();

	void updateGeometry(float deltaTime);
	void renderGeometry(glm::mat4 proj, glm::mat4 view);

	void setModel(glm::mat4 model);

	void applyTransformations();

	void checkStretch();

private:
	float springConstant;
	float dampConstant;
	float restLength;
	std::pair<ClothParticle*, ClothParticle*> endParticles;

	GLuint VAO;
	GLuint vbuffer, cbuffer, ibuffer;
	Shaders* springProgram;
	glm::mat4 model;

	static const GLfloat POSITIONS[][3];
	static const GLfloat NORMAL[][3];
	static const GLint INDICES[][3];
};


#endif // !CLOTH_SPRING_V2

