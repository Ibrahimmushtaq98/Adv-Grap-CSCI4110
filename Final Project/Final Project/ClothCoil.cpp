#include "ClothParticle.h"
#include "ShadersV2.h"
#include <algorithm>

constexpr GLfloat ClothCoil::POSITIONS[][3] = {
	{0.0, -0.5, 0.0},
	{0.0, 0.5, 0.0},
};

constexpr GLfloat ClothCoil::COLORS[][3] = {
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 1.0},
};

constexpr GLint ClothCoil::INDICES[][3] = {
	{1},
	{2},
};

ClothCoil::ClothCoil()
{
	glGenVertexArrays(1, &mVao);
	glGenBuffers(1, &mPositionBuffer);
	glGenBuffers(1, &mColorBuffer);
	glGenBuffers(1, &mIndexBuffer);

	glBindVertexArray(mVao);

	glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ClothCoil::POSITIONS), ClothCoil::POSITIONS, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ClothCoil::COLORS), ClothCoil::COLORS, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindVertexArray(0);

	coilShader = new Shaders((char*)"Shader/clothShader.vs", (char*)"Shader/clothShader.fs");
}

ClothCoil::~ClothCoil()
{
}

void ClothCoil::setSpringConstant(float springConstant)
{
	mSpringConstant = springConstant;
}

float ClothCoil::getSpringConstant() const
{
	return mSpringConstant;
}

void ClothCoil::setDampeningConstant(float dampeningConstant)
{
	mDampeningConstant = dampeningConstant;
}

float ClothCoil::getDampeningConstant() const
{
	return mDampeningConstant;
}

void ClothCoil::setEndParticles(std::pair<ClothParticle*, ClothParticle*> const& endParticles)
{
	mEndParticles = endParticles;
}

void ClothCoil::setEndParticles(ClothParticle* endA, ClothParticle* endB)
{
	mEndParticles = std::pair<ClothParticle*, ClothParticle*>(endA, endB);
}

std::pair<ClothParticle*, ClothParticle*> ClothCoil::getEndParticles() const
{
	return mEndParticles;
}

void ClothCoil::setRestLength(float restLength)
{
	mRestLength = restLength;
}

float ClothCoil::getRestLength() const
{
	return mRestLength;
}


void ClothCoil::updateGeometry(float t){
	float length = glm::length(mEndParticles.first->getPosition() - mEndParticles.second->getPosition());

	glm::vec3 forceSpring;
	if (length == 0)
	{
		forceSpring = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	else
	{
		glm::vec3 coilAxis = glm::normalize(mEndParticles.first->getPosition() - mEndParticles.second->getPosition());
		float deltaLength;
		if (length > mRestLength)
		{
			/*deltaLength = std::min(length - mRestLength, 1.1f*mRestLength - mRestLength);*/
			deltaLength = std::min<float>((length - mRestLength), ((1.1f) * mRestLength) - mRestLength);
		}
		else
		{
			deltaLength = std::max<float>((length - mRestLength), ((0.9f) * mRestLength) - mRestLength);
		}

		forceSpring = mSpringConstant * coilAxis * (deltaLength);
	}

	//Compute the dampening force
	glm::vec3 forceDampening1 = -mDampeningConstant * mEndParticles.first->getVelocity();
	glm::vec3 forceDampening2 = -mDampeningConstant * mEndParticles.second->getVelocity();

	mEndParticles.first->addForce(-forceSpring + forceDampening1);
	mEndParticles.second->addForce(forceSpring + forceDampening2);
}

void ClothCoil::checkStretch()
{
	ClothParticle* first = mEndParticles.first;
	ClothParticle* second = mEndParticles.second;

	if (first->isFixed() && second->isFixed())
	{
		//Can't do anything
		return;
	}
	float stretch = glm::length(first->getPosition() - second->getPosition());
	glm::vec3 firstToSecond = glm::normalize(second->getPosition() - first->getPosition());

	//if particles are too far from each other
	float stretchFactor = 1.0001f;
	if (stretchFactor * mRestLength < stretch)
	{
		float offset = stretch - stretchFactor * mRestLength;

		//If both particles are allowed to move
		if (!first->isFixed() && !second->isFixed())
		{
			offset *= 0.5f;

			//both particles must move offset closer to each other
			first->setPosition(first->getPosition() + offset * firstToSecond);
			second->setPosition(second->getPosition() - offset * firstToSecond);
			return;
		}

		if (!first->isFixed())
		{
			first->setPosition(first->getPosition() + offset * firstToSecond);
			return;
		}

		//second is not fixed
		second->setPosition(second->getPosition() - offset * firstToSecond);
		return;
	}

	//if particles are too close to each other
	else if (0.9999f * mRestLength > stretch)
	{
		float offset = 0.9999f * mRestLength - stretch;

		if (!first->isFixed() && !second->isFixed())
		{
			//both particles are allowed to move
			offset *= 0.50f;

			//both particles must move offset away from each other
			first->setPosition(first->getPosition() - offset * firstToSecond);
			second->setPosition(second->getPosition() + offset * firstToSecond);
			return;
		}

		if (!first->isFixed())
		{
			first->setPosition(first->getPosition() - offset * firstToSecond);
			return;
		}

		//second is not fixed
		second->setPosition(second->getPosition() + offset * firstToSecond);
		return;
	}
}

void ClothCoil::setModel(glm::mat4 const& model)
{
	this->model = model;
}

void ClothCoil::applyTransformations()
{
	//Resets the model matrices so that we can re-apply scaling and/or translations to them
	setModel(glm::mat4(1.0f));

	//Compute the axis of the coil and the length of the coil


	glm::vec3 coilAxis = mEndParticles.first->getPosition() - mEndParticles.second->getPosition();
	float coilLength = glm::length(coilAxis);

	//Scale the coil to its correct length
	this->model = (glm::scale(glm::mat4(1.0f), glm::vec3(1.0, coilLength, 1.0))) * this->model;

	if (coilLength > 0)
	{

		//Convert the coil axis to a unit vector
		coilAxis /= coilLength;

		//we rotate the entire spring so that it is algined with its correct axis
		glm::vec3 w = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), coilAxis);
		if (w != glm::vec3(0.0f, 0.0f, 0.0f))
		{
			float theta = acos(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), coilAxis));
			this->model= (glm::rotate(glm::mat4(1.0f), theta, w)) * this->model;
		}


	}

	//Move the spring to its correct location
	glm::vec3 midpoint = 0.5f * (mEndParticles.first->getPosition() + mEndParticles.second->getPosition());
	this->model = (glm::translate(glm::mat4(1.0f), midpoint)) * this->model;
}


void ClothCoil::renderGeometry(glm::mat4 const& projection, glm::mat4 const& view){
	coilShader->useShader();

	glm::mat4 modelViewProjection = projection * view * this->model;
	// glUniformMatrix4fv(mUniforms["ModelViewProjection"], 1, GL_FALSE, &modelViewProjection[0][0]);

	const GLint MODEL_VIEW_PROJECTION_UNIFORM_LOCATION = glGetUniformLocation(coilShader->getShaderID(), "ModelViewProjection");

	glUniformMatrix4fv(MODEL_VIEW_PROJECTION_UNIFORM_LOCATION, 1, GL_FALSE, &modelViewProjection[0][0]);

	//If I don't do this, my faces will be inside out
	//because I specified the cube vertices in clockwise order
	glFrontFace(GL_CW);

	glBindVertexArray(mVao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ClothCoil::INDICES), ClothCoil::INDICES, GL_STATIC_DRAW);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)0);

	glBindVertexArray(0);

}