#include "ClothSpringV2.h"

constexpr GLfloat ClothSpring::POSITIONS[][3] = {
	{0.0, -0.5, 0.0},
	{0.0, 0.5, 0.0},
};

constexpr GLfloat ClothSpring::NORMAL[][3] = {
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 1.0},
};

constexpr GLint ClothSpring::INDICES[][3] = {
	{1},
	{2},
};

ClothSpring::ClothSpring() {
}

void ClothSpring::applyTransformations() {
	this->setModel(glm::mat4(1.0f));

	glm::vec3 coilAxis = this->endParticles.first->getPosition() - this->endParticles.second->getPosition();
	float coilLength = glm::length(coilAxis);

	this->model = (glm::scale(glm::mat4(1.0f), glm::vec3(1.0, coilLength, 1.0))) * this->model;

	if (coilLength > 0) {
		coilAxis /= coilLength;

		//we rotate the entire spring so that it is algined with its correct axis
		glm::vec3 w = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), coilAxis);
		if (w != glm::vec3(0.0f, 0.0f, 0.0f)) {
			float theta = acos(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), coilAxis));
			this->model = (glm::rotate(glm::mat4(1.0f), theta, w)) * this->model;
		}

	}

	//Move the spring to its correct location
	glm::vec3 midpoint = 0.5f * (this->endParticles.first->getPosition() + this->endParticles.second->getPosition());
	this->model = (glm::translate(glm::mat4(1.0f), midpoint)) * this->model;
}

void ClothSpring::checkStretch(){
	ClothParticle* first = this->endParticles.first;
	ClothParticle* second = this->endParticles.second;

	if (first->isFixed() && second->isFixed()){
		return;
	}
	float stretch = glm::length(first->getPosition() - second->getPosition());
	glm::vec3 firstToSecond = glm::normalize(second->getPosition() - first->getPosition());

	float stretchFactor = 1.0001f;
	if (stretchFactor * this->restLength < stretch){
		float offset = stretch - stretchFactor * this->restLength;

		if (!first->isFixed() && !second->isFixed()){
			offset *= 0.5f;

			first->setPosition(first->getPosition() + offset * firstToSecond);
			second->setPosition(second->getPosition() - offset * firstToSecond);
			return;
		}

		if (!first->isFixed()){
			first->setPosition(first->getPosition() + offset * firstToSecond);
			return;
		}

		second->setPosition(second->getPosition() - offset * firstToSecond);
		return;
	}

	//if particles are too close to each other
	else if (0.9999f * this->restLength > stretch){
		float offset = 0.9999f * this->restLength - stretch;

		if (!first->isFixed() && !second->isFixed()){
			offset *= 0.50f;

			first->setPosition(first->getPosition() - offset * firstToSecond);
			second->setPosition(second->getPosition() + offset * firstToSecond);
			return;
		}

		if (!first->isFixed()){
			first->setPosition(first->getPosition() - offset * firstToSecond);
			return;
		}

		second->setPosition(second->getPosition() + offset * firstToSecond);
		return;
	}
}

void ClothSpring::updateGeometry(float deltaTime){
	float length = glm::length(this->endParticles.first->getPosition() - this->endParticles.second->getPosition());

	glm::vec3 forceSpring;
	if (length == 0){
		forceSpring = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	else{
		glm::vec3 coilAxis = glm::normalize(this->endParticles.first->getPosition() - this->endParticles.second->getPosition());
		float deltaLength;
		if (length > this->restLength){
			/*deltaLength = std::min(length - mRestLength, 1.1f*mRestLength - mRestLength);*/
			deltaLength = std::min<float>((length - this->restLength), ((1.1f) * this->restLength) - this->restLength);
		}
		else{
			deltaLength = std::max<float>((length - this->restLength), ((0.9f) * this->restLength) - this->restLength);
		}

		forceSpring = this->springConstant * coilAxis * (deltaLength);
	}

	//Compute the dampening force
	glm::vec3 forceDampening1 = -this->dampConstant * this->endParticles.first->getVelocity();
	glm::vec3 forceDampening2 = -this->dampConstant * this->endParticles.second->getVelocity();

	this->endParticles.first->addForce(-forceSpring + forceDampening1);
	this->endParticles.second->addForce(forceSpring + forceDampening2);
}

void ClothSpring::setSpringConstant(float springConstant){
	this->springConstant = springConstant;
}

float ClothSpring::getSpringConstant(){
	return this->springConstant;
}

void ClothSpring::setDampeningConstant(float dampConstant){
	this->dampConstant = dampConstant;
}

float ClothSpring::getDampeningConstant() {
	return this->dampConstant;
}

void ClothSpring::setEndParticles(std::pair<ClothParticle*, ClothParticle*> endParticles) {
	this->endParticles = endParticles;
}

void ClothSpring::setEndParticles(ClothParticle* endA, ClothParticle* endB) {
	this->endParticles = std::pair<ClothParticle*, ClothParticle*>(endA, endB);
}

std::pair<ClothParticle*, ClothParticle*> ClothSpring::getEndParticles() {
	return this->endParticles;
}

void ClothSpring::setRestLength(float restLength) {
	this->restLength = restLength;
}

float ClothSpring::getRestLength() {
	return this->restLength;
}

void ClothSpring::setModel(glm::mat4 model){
	this->model = model;
}