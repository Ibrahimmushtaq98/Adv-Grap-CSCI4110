#pragma once

#ifndef DIAMOND_SQUARE_H
#define DIAMOND_SQUARE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <random>
#include <iostream>
#include <ctime>
#include <vector>

class DiamondSquare {
public:
	int xheight = 0;
	int yheight = 0;
	int resolution = 0; // Number of GridPoints within x and y;
	int sizeVertexPos = 0;
	int sizeIndex = 0;

	GLfloat* vertexPositionData;
	GLdouble** heightMap;
	GLushort* indexData;

	GLfloat* normals;
	GLfloat* uv;
	GLfloat* triangles;

	std::default_random_engine generator;

	DiamondSquare(int xheight, int yheight, int n) {
		this->resolution = n;
		this->xheight = xheight;
		this->yheight = yheight;

		this->sizeVertexPos = this->resolution * this->resolution * 3;
		this->sizeIndex = ((this->resolution) * 2) * (this->resolution - 1) + (this->resolution - 2);

		//this->sizeVertexPos = this->resolution * this->resolution * 3 * 3 * 2;
		//this->sizeIndex = ((this->resolution) * (this->resolution)) * 3 * 2;

		this->vertexPositionData = new GLfloat[this->sizeVertexPos];
		this->indexData = new GLushort[this->sizeIndex];

		GenerateHeightMap();
		GenerateField();
	}

	void GenerateHeightMap() {

		//Generates the height map with initial value being 0
		this->heightMap = new GLdouble* [this->resolution];

		for (int i = 0; i < this->resolution; i++){
			this->heightMap[i] = new GLdouble[this->resolution];

			for (int j = 0; j < this->resolution; j++) {
				this->heightMap[i][j] = 0.0;
			}
		}


		this->heightMap[0][0] = GenerateValue(); //SOME RANDOM VALUE
		this->heightMap[0][this->resolution - 1] = GenerateValue(); //SOME RANDOM VALUE
		this->heightMap[this->resolution - 1][0] = GenerateValue(); //SOME RANDOM VALUE
		this->heightMap[this->resolution - 1][this->resolution - 1] = GenerateValue(); //SOME RANDOM VALUE
		std::cout << "Height Map without any DS algorithim" << std::endl;
		printHeightMap();

		this->diamondSquare();
		std::cout << "Height Map with DS algorithim (NOTE some of the diamond value got some new rand val)" << std::endl;
		printHeightMap();

	}

	void printHeightMap() {
		for (int i = 0; i < this->resolution; i++) {
			for (int j = 0; j < this->resolution; j++) {
				std::cout << this->heightMap[i][j] << " ";
			}
			std::cout << std::endl;
		}

		std::cout << "******************************" << std::endl;
	}

	void GenerateField() {

		int i = 0;
		for (int z = 0; z < this->resolution; z++) {

			float zPos = ((float)z / (this->resolution - 1) - 0.5f) * this->yheight;
			for (int x = 0; x < this->resolution; x++) {

				float xPos = ((float)x / (this->resolution - 1) - 0.5f) * this->xheight;
				float yPos = (float)0.0f;
				//std::cout << "x: " << xPos << " y: " << yPos << " z: " << zPos << std::endl;
				this->vertexPositionData[i++] = (float)xPos;
				this->vertexPositionData[i++] = (float)this->heightMap[z][x];
				this->vertexPositionData[i++] = (float)zPos;
			}
		}
		i = 0;

		for (int z = 0; z < this->resolution - 1; z++) {
			if ((z % 2) == 0) {
				int x;
				for (x = 0; x < this->resolution; x++) {
					this->indexData[i++] = x + (z * this->resolution);
					this->indexData[i++] = x + (z * this->resolution) + this->resolution;
				}
				if (z != this->resolution - 2) {
					this->indexData[i++] = --x + (z * this->resolution);
				}
			}
			else {
				int x;
				for (x = this->resolution - 1; x >= 0; x--) {
					this->indexData[i++] = x + (z * this->resolution);
					this->indexData[i++] = x + (z * this->resolution) + this->resolution;
				}
				if (z != this->resolution - 2) {
					this->indexData[i++] = ++x + (z * this->resolution);
				}

			}
		}
	}

private:

	double GenerateValue() {
		std::normal_distribution<double> distribution(5.0, 2.0);
		double number = distribution(generator);
		return number;
	}

	void diamondSquare() {
		int sideL = this->resolution / 2;

		diamondSteps(this->resolution);
		squareSteps(this->resolution);

		while (sideL >= 2){
			diamondSteps(sideL + 1);
			squareSteps(sideL + 1);
			sideL /= 2;
		}
	}

	void squareSteps(int sideS) {
		int halfLength = sideS / 2;

		for (int y = 0; y < this->resolution / (sideS - 1); y++){
			for (int x = 0; x < this->resolution / (sideS - 1); x++){
				// Top
				average(x * (sideS - 1) + halfLength, y * (sideS - 1), sideS);
				// Right
				average((x + 1) * (sideS - 1), y * (sideS - 1) + halfLength, sideS);
				// Bottom
				average(x * (sideS - 1) + halfLength, (y + 1) * (sideS - 1), sideS);
				// Left
				average(x * (sideS - 1), y * (sideS - 1) + halfLength, sideS);
			}
		}
	}

	void diamondSteps(int sideS) {
		int halfSide = sideS / 2;

		for (int z = 0; z < this->resolution / (sideS - 1);z++){
			for (int x = 0; x < this->resolution / (sideS - 1); x++){
				int cenx = x * (sideS - 1) + halfSide;
				int ceny = z * (sideS - 1) + halfSide;

				double avg = (this->heightMap[x * (sideS - 1)][z * (sideS - 1)] +
					this->heightMap[x * (sideS - 1)][(z + 1) * (sideS - 1)] +
					this->heightMap[(x + 1) * (sideS - 1)][z * (sideS - 1)] +
					this->heightMap[(x + 1) * (sideS - 1)][(z + 1) * (sideS - 1)])
					/ 4.0f;

				this->heightMap[cenx][ceny] = avg + GenerateValue();
			}
		}
	}

	void average(int x, int z, int sideS){
		double counter = 0;
		double avg = 0;

		int halfSide = sideS / 2;

		if (x != 0){
			counter += 1.0f;
			avg += this->heightMap[z][x - halfSide];
		}
		if (z != 0){
			counter += 1.0f;
			avg += this->heightMap[z - halfSide][x];
		}
		if (x != this->resolution - 1){
			counter += 1.0f;
			avg += this->heightMap[z][x + halfSide];
		}
		if (z != this->resolution - 1){
			counter += 1.0f;
			avg += this->heightMap[z + halfSide][x];
		}

		this->heightMap[z][x] = (avg / counter);
	}
};
#endif // !DIAMOND_SQUARE_H