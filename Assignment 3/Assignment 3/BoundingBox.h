#pragma once

#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/gtx/string_cast.hpp>

#include <string>
#include <iostream>
#include <vector>
#include "tiny_obj_loader.h"
#include "tribes.h"

struct BoundBox {
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 orig;
	float speed;
};

class BoundingBox {
	const float STEPS_SIZE= 0.673076;
public:
	std::vector<BoundBox> bb;

	BoundingBox(char* filelocation, bool isGround) {
		initModel(filelocation, isGround);
	}

	bool checkCollision(BoundBox& b1, BoundBox& b2) {

		if ((b1.min.x <= b2.max.x && b1.max.x >= b2.min.x) &&
			(b1.min.y <= b2.max.y && b1.max.y >= b2.min.y) &&
			(b1.min.z <= b2.max.z && b1.max.z >= b2.min.z)) {
			return true;
		}

		return false;
	}

private:
	void initModel(char* filelocation, bool isGround) {
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		int i, count = 0;

		std::string err = tinyobj::LoadObj(shapes, materials, filelocation, 0);
		if (!err.empty()) {
			std::cerr << err << std::endl;
			return;
		}

		int nv = (int)shapes[0].mesh.positions.size();
		float x,y,z;
		if (isGround) {
			for (i = 0; i < nv;) {
				x = shapes[0].mesh.positions[i++];
				y = shapes[0].mesh.positions[i++];
				z = shapes[0].mesh.positions[i++];
				glm::vec3 vert = glm::vec3(x, y, z);

				if (y > 0){ 
					BoundBox b;
					b.min = glm::vec3(x - STEPS_SIZE, 0.0f, z - STEPS_SIZE);
					b.orig = vert;
					b.max = glm::vec3(x + STEPS_SIZE, y, z + STEPS_SIZE);
					bb.push_back(b);
				}

			}
		}
		else {
			glm::vec3 min = glm::vec3(shapes[0].mesh.positions[0],
										shapes[0].mesh.positions[1],
										shapes[0].mesh.positions[2]);
			glm::vec3 max = min;
			for (i = 3; i < nv;) {
				x = shapes[0].mesh.positions[i++];
				y = shapes[0].mesh.positions[i++];
				z = shapes[0].mesh.positions[i++];
				glm::vec3 vert = glm::vec3(x, y, z);
				if (x < min.x) { min.x = x; }
				if (x > max.x) { max.x = x; }
				if (y < min.x) { min.y = y; }
				if (y > max.x) { max.y = y; }
				if (z < min.x) { min.z = z; }
				if (z > max.x) { max.z = z; }
			}
			BoundBox b;
			b.max = max;
			b.min = min;
			bb.push_back(b);
		}
		
	}

	//void draw_bbox(struct mesh* mesh) {
	//	glm::vec3 size = glm::vec3(max_x - min_x, max_y - min_y, max_z - min_z);
	//	glm::vec3 center = glm::vec3((min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2);
	//	glm::mat4 transform = glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);
	//}
};
#endif

