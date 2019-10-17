#pragma once

#ifndef TERRAIN_MESH_H
#define TERRAIN_MESH_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "DiamondSquare.h"

class TerrainMesh {
public:
	std::vector<glm::vec3> trNormals;

	std::vector<GLfloat> mesh, normals;

	int resN;

	TerrainMesh(int res) {
		this->resN = res;
	}

	void generateTriangleMesh(GLdouble** heightMap) {

	}
	//void GenerateField() {

	//	int i = 0;
	//	for (int z = 0; z < this->resolution; z++) {

	//		float zPos = ((float)z / (this->resolution - 1) - 0.5f) * this->yheight;
	//		for (int x = 0; x < this->resolution; x++) {

	//			float xPos = ((float)x / (this->resolution - 1) - 0.5f) * this->xheight;
	//			float yPos = (float)0.0f;
	//			//std::cout << "x: " << xPos << " y: " << yPos << " z: " << zPos << std::endl;
	//			this->vertexPositionData[i++] = (float)xPos;
	//			this->vertexPositionData[i++] = (float)this->heightMap[z][x];
	//			this->vertexPositionData[i++] = (float)zPos;
	//		}
	//	}
	//	i = 0;

	//	for (int z = 0; z < this->resolution - 1; z++) {
	//		if ((z % 2) == 0) {
	//			int x;
	//			for (x = 0; x < this->resolution; x++) {
	//				this->indexData[i++] = x + (z * this->resolution);
	//				this->indexData[i++] = x + (z * this->resolution) + this->resolution;
	//			}
	//			if (z != this->resolution - 2) {
	//				this->indexData[i++] = --x + (z * this->resolution);
	//			}
	//		}
	//		else {
	//			int x;
	//			for (x = this->resolution - 1; x >= 0; x--) {
	//				this->indexData[i++] = x + (z * this->resolution);
	//				this->indexData[i++] = x + (z * this->resolution) + this->resolution;
	//			}
	//			if (z != this->resolution - 2) {
	//				this->indexData[i++] = ++x + (z * this->resolution);
	//			}

	//		}
	//	}

	//	std::cout << "I = " << i << std::endl;
	//}

};
#endif //TERRAIN_MESH_H