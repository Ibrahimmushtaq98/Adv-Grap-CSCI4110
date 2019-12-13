#ifndef CLOTH_PARTICLE
#define CLOTH_PARTICLE

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include "ShadersV2.h"


class ClothParticle {
	public:
		ClothParticle();

		void updateGeometry(float deltaTime);
		void renderGeometry(glm::mat4 proj, glm::mat4 view);

		//getter and setters
		void setMass(float m);
		float getMass();

		void setDampConstant(float d);
		float getDampConstant();

		void setPosition(glm::vec3 pos);
		glm::vec3 getPosition();

		void setVelocity(glm::vec3 veloc);
		glm::vec3 getVelocity();

		void setAcceleration(glm::vec3 accel);
		glm::vec3 getAcceleration();

		void setFixed(bool fix);
		bool isFixed();

		void addForce(glm::vec3 force);
		void setModel(glm::mat4 model);
		void applyTransformations();

	private:
		float mass;
		glm::vec3 position, velocity, acceleration;
		glm::vec3 tmpPos;

		bool posFixed;
		glm::mat4 model;
		glm::vec3 netForce, normalForce;

		float dampConstant;

		GLuint particleVAO;
		GLuint vbuffer, ibuffer, cbuffer;
		Shaders* particleProgram;

		static const GLfloat POSITIONS[][3];
		static const GLfloat NORMAL[][3];
		static const GLint INDICES[][1];

		glm::vec3 computeVelocity(float deltaTime, glm::vec3 velocity);
		glm::vec3 computeVelocity(float deltaTime);
		glm::vec3 computeAcceleration(float deltaTime, glm::vec3 velocity);
		glm::vec3 computeAcceleration(glm::vec3 position, glm::vec3 velocity);
};


#endif // !CLOTH_PARTICLE
