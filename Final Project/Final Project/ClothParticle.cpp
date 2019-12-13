#include "ClothParticle.h"
#include "ShadersV2.h"


constexpr GLfloat ClothParticle::POSITIONS[][3] = {
	{0.0, 0.0, 0.0}
};

constexpr GLfloat ClothParticle::NORMAL[][3] = {
	{1.0, 0.0, 0.0},
};

constexpr GLint ClothParticle::INDICES[][1] = {
	{1}
};

ClothParticle::ClothParticle(){
	//int vPosition, vNormal;

	//glGenVertexArrays(1, &this->particleVAO);
	//glGenBuffers(1, &this->vbuffer);
	//glGenBuffers(1, &this->cbuffer);
	//glGenBuffers(1, &this->ibuffer);
	//glBindVertexArray(this->particleVAO);

	//glGenBuffers(1, &this->vbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(ClothParticle::POSITIONS), ClothParticle::POSITIONS, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	//glGenBuffers(1, &this->cbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, this->cbuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(ClothParticle::NORMAL), ClothParticle::NORMAL, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	//particleProgram = new Shaders((char*)"Shader/clothShader.vs", (char*)"Shader/clothShader.fs");

	//particleProgram->useShader();
	//vPosition = glGetAttribLocation(particleProgram->getShaderID(), "vPosition");
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	//glEnableVertexAttribArray(vPosition);
	//vNormal = glGetAttribLocation(particleProgram->getShaderID(), "vNormal");
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	//glEnableVertexAttribArray(vNormal);
}


void ClothParticle::updateGeometry(float deltaTime) {
	if (this->isFixed()) {
		this->velocity = glm::vec3(0.0, 0.0, 0.0);
		this->acceleration = glm::vec3(0.0, 0.0, 0.0);
		this->netForce = glm::vec3(0.0, 0.0, 0.0);;

		this->applyTransformations();
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

	applyTransformations();

	this->position = newPositon;
	this->velocity = newVelocity;
	this->netForce = glm::vec3(0.0f, 0.0f, 0.0f);
}

void ClothParticle::renderGeometry(glm::mat4 proj, glm::mat4 view) {
	//particleProgram->useShader();

	//particleProgram->setMat4("modelView", view);
	//particleProgram->setMat4("model", this->model);
	//particleProgram->setMat4("projection", proj);

	//glFrontFace(GL_CW);
	//glPointSize(2.0f);

	//glBindVertexArray(this->particleVAO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibuffer);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ClothParticle::INDICES), ClothParticle::INDICES, GL_STATIC_DRAW);
	//glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, (void*)0);

	this->particleProgram->useShader();

	glm::mat4 modelViewProjection = proj * view * this->model;
	const GLint MODEL_VIEW_PROJECTION_UNIFORM_LOCATION = glGetUniformLocation(this->particleProgram->getShaderID(), "ModelViewProjection");
	glUniformMatrix4fv(MODEL_VIEW_PROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, &modelViewProjection[0][0]);

	//If I don't do this, my faces will be inside out
	//because I specified the cube vertices in clockwise order
	glFrontFace(GL_CW);
	glPointSize(1.5f);

	glBindVertexArray(this->particleVAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ClothParticle::INDICES), ClothParticle::INDICES, GL_STATIC_DRAW);
	glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, (void*)0);

	glBindVertexArray(0);
}
//--------------GETTER AND SETTERS---------------->
void ClothParticle::setFixed(bool fixed) {
	this->posFixed = fixed;
}

bool ClothParticle::isFixed() {
	return this->posFixed;
}

float ClothParticle::getMass() {
	return this->mass;
}

void ClothParticle::setMass(float m) {
	this->mass = m;
}

void ClothParticle::setPosition(glm::vec3 pos) {
	this->position = pos;
}

glm::vec3 ClothParticle::getPosition() {
	return this->position;
}

void ClothParticle::setVelocity(glm::vec3 veloc) {
	this->velocity = veloc;
}

glm::vec3 ClothParticle::getVelocity() {
	return this->velocity;
}

void ClothParticle::setAcceleration(glm::vec3 accel) {
	this->acceleration = accel;
}

glm::vec3 ClothParticle::getAcceleration() {
	return this->acceleration;
}

float ClothParticle::getDampConstant() {
	return this->dampConstant;
}

void ClothParticle::setDampConstant(float d) {
	this->dampConstant = d;
}

glm::vec3 ClothParticle::computeVelocity(float deltaTime, glm::vec3 velocity) {
	return velocity + deltaTime * this->acceleration;
}

glm::vec3 ClothParticle::computeVelocity(float deltaTime) {
	return computeVelocity(deltaTime, this->velocity);
}

glm::vec3 ClothParticle::computeAcceleration(float deltaTime, glm::vec3 velocity) {
	glm::vec3 position = this->position + velocity * deltaTime;
	return computeAcceleration(position, velocity);
}

glm::vec3 ClothParticle::computeAcceleration(glm::vec3 position, glm::vec3 velocity) {
	const float g = 9.81;
	glm::vec3 forceGravity(0.0f, -g * this->mass, 0.0f);

	this->netForce += forceGravity + this->normalForce;

	return this->netForce / this->mass;
}

void ClothParticle::addForce(glm::vec3 force) {
	this->netForce += force;
}

void ClothParticle::setModel(glm::mat4 model) {
	this->model = model;
}

void ClothParticle::applyTransformations() {
	this->setModel(glm::mat4(1.0f));
	this->model = glm::translate(glm::mat4(1.0f), this->position) * this->model;
}