#pragma once
#include <glm/glm.hpp>
#include <string>
#include "BoundingBox.h"

struct Tribes {
	std::string tribeClan;
	glm::mat4 position;
	GLfloat speed;
	glm::vec3 velocity;
	GLfloat acceleration;
	glm::vec4 color;
	glm::vec3 min;
	glm::vec3 max;
};