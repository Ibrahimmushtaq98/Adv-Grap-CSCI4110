#pragma once
#pragma once
#ifndef CLOTH_PARTICLE
#define CLOTH_PARTICLE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ClothParticle
{
public:
	ClothParticle() {}

	void setVelocity(glm::vec3 vel) {
		this->velocity = vel;
	}

	glm::vec3 getVelocity() {
		return this->velocity;
	}

	void setPosition(glm::vec3 pos) {
		this->position = pos;
	}

	glm::vec3 getPosition() {
		return this->position;
	}

	void setMass(float mass) {
		this->mass = mass;
	}

	float getMass() {
		return this->mass;
	}

	void setFixed(bool fix) {
		this->isFixed = fix;
	}

	bool getFixed() {
		return this->isFixed;
	}

	void addForce(glm::vec3 force) {
		this->netForce += force;
	}
	void setModel(glm::mat4 model) {
		this->model = model;
	}

	void setAcceleration(glm::vec3 accel) {
		this->acceleration = accel;
	}

	glm::vec3 getAcceleration() {
		return this->acceleration;
	}

	float getDampConstant() {
		return this->dampConstant;
	}

	void setDampConstant(float d) {
		this->dampConstant = d;
	}

	glm::vec3 computeVelocity(float deltaTime, glm::vec3 velocity) {
		return velocity + deltaTime * this->acceleration;
	}

	glm::vec3 computeVelocity(float deltaTime) {
		return computeVelocity(deltaTime, this->velocity);
	}

	glm::vec3 computeAcceleration(float deltaTime, glm::vec3 velocity) {
		glm::vec3 position = this->position + velocity * deltaTime;
		return computeAcceleration(position, velocity);
	}

	glm::vec3 computeAcceleration(glm::vec3 position, glm::vec3 velocity) {
		const float g = 9.81;
		glm::vec3 forceGravity(0.0f, -g * this->mass, 0.0f);
		glm::vec3 forceWind = this->mass * glm::vec3(0.0,0.0,0.0);

		this->netForce += forceGravity + this->normalForce +forceWind;

		return this->netForce / this->mass;
	}
	void updateGeometry(float deltaTime) {
		if (this->getFixed()) {
			this->velocity = glm::vec3(0.0, 0.0, 0.0);
			this->acceleration = glm::vec3(0.0, 0.0, 0.0);
			this->netForce = glm::vec3(0.0, 0.0, 0.0);;

			return;
		}

		glm::vec3 newPositon;
		glm::vec3 newVelocity;

		if (!false) {
			glm::vec3 v1 = deltaTime * this->computeVelocity(0);
			glm::vec3 v2 = deltaTime * this->computeVelocity(0.5 * deltaTime);
			glm::vec3 v3 = deltaTime * this->computeVelocity(0.5 * deltaTime);
			glm::vec3 v4 = deltaTime * this->computeVelocity(deltaTime);
			newPositon = this->position + (1.0f / 6.0f) * (v1 + 2.0f * v2 + 2.0f * v3 + v4);

			glm::vec3 a1 = deltaTime * this->computeAcceleration(0, this->velocity);
			glm::vec3 a2 = deltaTime * this->computeVelocity(0.5 * deltaTime, this->velocity + 0.5f * a1);
			glm::vec3 a3 = deltaTime * this->computeVelocity(0.5 * deltaTime, this->velocity + 0.5f * a2);
			glm::vec3 a4 = deltaTime * this->computeVelocity(deltaTime, this->velocity + a3);
			newVelocity = this->velocity + (1.0f / 6.0f) * (a1 + 2.0f * a2 + 2.0f * a3 + a4);

			this->acceleration = (newVelocity - this->velocity) / deltaTime;
		}
		else {
			glm::vec3 a1 = computeAcceleration(0, this->velocity);
			newPositon = this->position + this->velocity * deltaTime + 0.5f * a1 * deltaTime * deltaTime;
			newVelocity = this->velocity + 0.5f * (a1 + computeAcceleration(deltaTime, computeVelocity(deltaTime))) * deltaTime;
		}

		this->position = newPositon;
		this->velocity = newVelocity;
		this->netForce = glm::vec3(0.0f, 0.0f, 0.0f);
	}

private:
	glm::vec3 velocity, acceleration;;
	glm::vec3 position;
	glm::mat4 model;
	glm::vec3 netForce, normalForce;
	float dampConstant;

	float mass;
	bool isFixed;
};

#endif // !CLOTH_SPRING

