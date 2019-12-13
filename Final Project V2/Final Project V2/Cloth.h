#pragma once

#ifndef CLOTH
#define CLOTH

#include <GL/glew.h>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "ClothParticle.h"
#include "ClothSpring.h"

class Cloth{
public:
	Cloth(float w, float h, float n, float m) {
		this->width = w;
		this->height = h;
		this->gridPoint = n;
		this->mass = m;

		this->particleHeight = this->width * this->gridPoint;
		this->particleWidth = this->height * this->gridPoint;
		this->particleNum = this->particleHeight * this->particleWidth;
		this->numVertices = (this->particleWidth - 1) * (this->particleHeight - 1) * 6;

		this->vertexPositions = std::vector<glm::vec3>(this->particleNum);
		this->vertexNormals = std::vector<glm::vec3>(this->particleNum);
		this->vertexTextureCoords = std::vector<glm::vec2>(this->particleNum);
		this->vertexIndices = std::vector<GLint>(this->numVertices);

		this->particle = std::vector<ClothParticle>(this->particleNum);
		this->model = glm::mat3(1.0f);

		int currentParticle = 0;
		int i, j;

		for (i = 0; i < this->particleHeight; i++) {
			for (j = 0; j < this->particleWidth; j++) {
				int subscripts = i * w + 3 * j;

				ClothParticle part = ClothParticle();
				part.setPosition(glm::vec3(j / this->gridPoint, 0.0f, i / this->gridPoint));
				part.setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
				part.setMass(this->mass);

				if ((i == this->particleHeight - 1 && j == 0) ||(i == particleHeight - 1 && j == particleWidth - 1)){
					part.setFixed(true);
				}
				else {
					part.setFixed(false);
				}

				this->particle[currentParticle] = part;
				currentParticle++;
			}
		}

		//TODO ADD FOR SPRING

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				int current = i * w + j;

				ClothParticle part = this->particle[current];

				float structSpringConst = 1.0f;
				float structDampConst = 0.002f;
				float shearSpringConst = 1.0f;
				float shearDampConst = 0.0f;
				float bendSpringConst = 1.0f;
				float bendDampConst = 0.0f;

				int left = current - 1;
				int right = current + 1;
				int top = current - h;
				int bottom = current + h;

				int topLeft = top - 1;
				int topRight = top + 1;
				int bottomLeft = bottom - 1;
				int bottomRight = bottom + 1;
				int rightRight = right + 1;
				int bottomBottom = bottom + h;

				//Setup the structural spring

				if (j < (w - 1)) {
					ClothParticle tmpP = this->particle[right];
					ClothSpring coil = ClothSpring();
					coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
					coil.setEndParticles(&part, &tmpP);
					coil.setSpringConstant(structSpringConst);
					coil.setDampeningConstant(structDampConst);
					this->structural.push_back(coil);
				}
				if (i < h - 1) {
					ClothParticle tmpP = this->particle[bottom];
					ClothSpring coil = ClothSpring();
					coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
					coil.setEndParticles(&part, &tmpP);
					coil.setSpringConstant(structSpringConst);
					coil.setDampeningConstant(structDampConst);
					this->structural.push_back(coil);
				}

				//Setup the shear spring

				if (i > 0 && j < w - 1) {
					ClothParticle tmpP = this->particle[topRight];
					ClothSpring coil = ClothSpring();
					coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
					coil.setEndParticles(&part, &tmpP);
					coil.setSpringConstant(shearSpringConst);
					coil.setDampeningConstant(shearDampConst);
					this->shears.push_back(coil);
				}
				if (i < h - 1 && j < w - 1) {
					ClothParticle tmpP = this->particle[bottomRight];
					ClothSpring coil = ClothSpring();
					coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
					coil.setEndParticles(&part, &tmpP);
					coil.setSpringConstant(shearSpringConst);
					coil.setDampeningConstant(shearDampConst);
					this->shears.push_back(coil);
				}

				//Setup the bend

				if (j < w - 2) {
					ClothParticle tmpP = this->particle[rightRight];
					ClothSpring coil = ClothSpring();
					coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
					coil.setEndParticles(&part, &tmpP);
					coil.setSpringConstant(bendSpringConst);
					coil.setDampeningConstant(bendDampConst);
					this->bend.push_back(coil);
				}
				if (i < h - 2) {
					ClothParticle tmpP = this->particle[bottomBottom];
					ClothSpring coil = ClothSpring();
					coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
					coil.setEndParticles(&part, &tmpP);
					coil.setSpringConstant(bendSpringConst);
					coil.setDampeningConstant(bendDampConst);
					this->bend.push_back(coil);
				}
			}
		}

		this->clothProgram = new Shaders((char*)"Shader/clothShadervs.vert", (char*)"Shader/clothShaderfs.frag");
		//bindInformation();
	}
	void updateGeometry(float deltaTime) {
		//For each coil in the cloth, update its geometry
		//which consists of computing the forces acting
		//on the cloth particles attached to both ends
		//of the coil

		float newTime = deltaTime / 10.0f;

		for (int i = 0; i < 10; ++i) {
			for (auto& coil : this->structural) {
				coil.updateGeometry(deltaTime);
			}
			for (auto& coil : this->shears) {
				coil.updateGeometry(deltaTime);
			}
			for (auto& coil : this->bend) {
				coil.updateGeometry(deltaTime);
			}

			//Now update the positions and velocities
			//of the cloth particles
			for (auto& particle : this->particle) {
				particle.updateGeometry(deltaTime);
			}

			for (int i = 0; i < 4; ++i) {
				for (auto& coil : this->structural) {
					coil.checkStretch();
				}
				for (auto& coil : this->shears) {
					coil.checkStretch();
				}
			}
		}

		//bindInformation();
	}

	void bindInformation() {
		int i,j;
		int vPosition, vNormal, vTextCoord;

		for (i = 0; i <this->particleHeight ; i++){
			for (j = 0; j < this->particleWidth; j++)
			{
				int current = i * this->particleHeight + j;
				this->vertexPositions[current] = this->particle[current].getPosition();
				this->vertexTextureCoords[current] = glm::vec2((float)j / (this->particleWidth - 1), (float)i / (this->particleHeight - 1));
			}
		}


		int index = 0;
		for (i = 0; i < this->particleHeight - 1; ++i){
			for (int j = 0; j < this->particleWidth - 1; ++j){
				glm::vec3 normal;

				int current = i * this->particleWidth + j;
				int right = current + 1;
				int bottom = current + this->particleWidth;
				int bottomRight = bottom + 1;

				this->vertexIndices[index] = current;
				this->vertexIndices[index + 1] = bottom;
				this->vertexIndices[index + 2] = bottomRight;

				normal = glm::normalize(
					glm::cross(this->vertexPositions[bottomRight] - this->vertexPositions[current], this->vertexPositions[bottom] - this->vertexPositions[current])
				);
				this->vertexNormals[current] += normal;
				this->vertexNormals[bottom] += normal;
				this->vertexNormals[bottomRight] += normal;


				this->vertexIndices[index + 3] = current;
				this->vertexIndices[index + 4] = bottomRight;
				this->vertexIndices[index + 5] = right;

				normal = glm::normalize(
					glm::cross(this->vertexPositions[right] - this->vertexPositions[current], this->vertexPositions[bottomRight] - this->vertexPositions[current])
				);
				this->vertexNormals[current] += normal;
				this->vertexNormals[bottomRight] += normal;
				this->vertexNormals[right] += normal;

				index += 6;
			}
		}

		for (i = 0; i < this->vertexPositions.size(); i++) {
			//std::cout << glm::to_string(this->vertexPositions[i]) << "\n";
		}

		for (i = 0; i < this->vertexIndices.size(); i++) {
			//std::cout << this->vertexIndices[i] << "\n";
		}

		glGenVertexArrays(1, &this->objectVAO);
		glBindVertexArray(this->objectVAO);

		int nv, nn, nt, ni;
		GLfloat* vertices;
		GLfloat* normals;
		GLfloat* textCoord;
		GLuint* indexs;
		int count = 0;

		nv = this->vertexPositions.size() * 3;
		vertices = new GLfloat[nv];
		for (i = 0; i < nv;) {
			vertices[i++] = this->vertexPositions[count].x;
			vertices[i++] = this->vertexPositions[count].y;
			vertices[i++] = this->vertexPositions[count].z;
			count++;
		}

		nn = this->vertexNormals.size() * 3;
		normals = new GLfloat[nn];
		count = 0;
		for (i = 0; i < nn; i++) {
			normals[i++] = this->vertexNormals[count].x;
			normals[i++] = this->vertexNormals[count].y;
			normals[i++] = this->vertexNormals[count].z;
		}

		nt = this->vertexTextureCoords.size() * 2;
		textCoord = new GLfloat[nt];
		count = 0;
		for (i = 0; i < nt; i++) {
			textCoord[i++] = this->vertexNormals[count].x;
			textCoord[i++] = this->vertexNormals[count].y;
		}

		ni = this->vertexIndices.size();
		indexs = new GLuint[ni];
		for (int i = 0; i < ni; i++) {
			indexs[i] = this->vertexIndices[i];
		}


		glGenBuffers(1, &this->vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, (nv) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &this->normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, (nn) * sizeof(GLfloat), normals, GL_STATIC_DRAW);

		glGenBuffers(1, &this->textcoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, this->textcoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, (nt) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &this->indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni * sizeof(GLuint), indexs, GL_STATIC_DRAW);

		this->clothProgram->useShader();
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
		vPosition = glGetAttribLocation(this->clothProgram->getShaderID(), "vPosition");
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(vPosition);

		glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
		vNormal = glGetAttribLocation(this->clothProgram->getShaderID(), "vNormal");
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)((nn) * sizeof(normals)));
		glEnableVertexAttribArray(vNormal);

		glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
		vTextCoord = glGetAttribLocation(this->clothProgram->getShaderID(), "vTextCoord");
		glVertexAttribPointer(vTextCoord, 2, GL_FLOAT, GL_FALSE, 0, (void*)((nt) * sizeof(textCoord)));
		glEnableVertexAttribArray(vTextCoord);
		
	}

	void renderGeometry(glm::mat4 proj, glm::mat4 view) {
		int i, j;
		int vPosition, vNormal, vTextCoord;

		for (i = 0; i < this->particleHeight; i++) {
			for (j = 0; j < this->particleWidth; j++)
			{
				int current = i * this->particleHeight + j;
				this->vertexPositions[current] = this->particle[current].getPosition();
				this->vertexTextureCoords[current] = glm::vec2((float)j / (this->particleWidth - 1), (float)i / (this->particleHeight - 1));
			}
		}


		int index = 0;
		for (i = 0; i < this->particleHeight - 1; ++i) {
			for (int j = 0; j < this->particleWidth - 1; ++j) {
				glm::vec3 normal;

				int current = i * this->particleWidth + j;
				int right = current + 1;
				int bottom = current + this->particleWidth;
				int bottomRight = bottom + 1;

				this->vertexIndices[index] = current;
				this->vertexIndices[index + 1] = bottom;
				this->vertexIndices[index + 2] = bottomRight;

				normal = glm::normalize(
					glm::cross(this->vertexPositions[bottomRight] - this->vertexPositions[current], this->vertexPositions[bottom] - this->vertexPositions[current])
				);
				this->vertexNormals[current] += normal;
				this->vertexNormals[bottom] += normal;
				this->vertexNormals[bottomRight] += normal;


				this->vertexIndices[index + 3] = current;
				this->vertexIndices[index + 4] = bottomRight;
				this->vertexIndices[index + 5] = right;

				normal = glm::normalize(
					glm::cross(this->vertexPositions[right] - this->vertexPositions[current], this->vertexPositions[bottomRight] - this->vertexPositions[current])
				);
				this->vertexNormals[current] += normal;
				this->vertexNormals[bottomRight] += normal;
				this->vertexNormals[right] += normal;

				index += 6;
			}
		}

		for (i = 0; i < this->particleNum; ++i){
			this->vertexNormals[i] = normalize(this->vertexNormals[i]);
		}

		for (i = 0; i < this->vertexPositions.size(); i++) {
			//std::cout << glm::to_string(this->vertexPositions[i]) << "\n";
		}

		for (i = 0; i < this->vertexIndices.size(); i++) {
			//std::cout << this->vertexIndices[i] << "\n";
		}

		glGenVertexArrays(1, &this->objectVAO);
		glBindVertexArray(this->objectVAO);

		int nv, nn, nt, ni;
		GLfloat* vertices;
		GLfloat* normals;
		GLfloat* textCoord;
		GLuint* indexs;
		int count = 0;

		nv = this->vertexPositions.size() * 3;
		vertices = new GLfloat[nv];
		for (i = 0; i < nv;) {
			vertices[i++] = this->vertexPositions[count].x;
			vertices[i++] = this->vertexPositions[count].y;
			vertices[i++] = this->vertexPositions[count].z;
			count++;
		}

		nn = this->vertexNormals.size() * 3;
		normals = new GLfloat[nn];
		count = 0;
		for (i = 0; i < nn; i++) {
			normals[i++] = this->vertexNormals[count].x;
			normals[i++] = this->vertexNormals[count].y;
			normals[i++] = this->vertexNormals[count].z;
		}

		nt = this->vertexTextureCoords.size() * 2;
		textCoord = new GLfloat[nt];
		count = 0;
		for (i = 0; i < nt; i++) {
			textCoord[i++] = this->vertexNormals[count].x;
			textCoord[i++] = this->vertexNormals[count].y;
		}

		ni = this->vertexIndices.size();
		indexs = new GLuint[ni];
		for (int i = 0; i < ni; i++) {
			indexs[i] = this->vertexIndices[i];
		}


		glGenBuffers(1, &this->vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, (nv) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &this->normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, (nn) * sizeof(GLfloat), normals, GL_STATIC_DRAW);

		glGenBuffers(1, &this->textcoordBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, this->textcoordBuffer);
		glBufferData(GL_ARRAY_BUFFER, (nt) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &this->indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni * sizeof(GLuint), indexs, GL_STATIC_DRAW);

		this->clothProgram->useShader();
		glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);
		vPosition = glGetAttribLocation(this->clothProgram->getShaderID(), "vPosition");
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(vPosition);

		glBindBuffer(GL_ARRAY_BUFFER, this->normalBuffer);
		vNormal = glGetAttribLocation(this->clothProgram->getShaderID(), "vNormal");
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)((nn) * sizeof(normals)));
		glEnableVertexAttribArray(vNormal);

		glBindBuffer(GL_ARRAY_BUFFER, this->textcoordBuffer);
		vTextCoord = glGetAttribLocation(this->clothProgram->getShaderID(), "vTextCoord");
		glVertexAttribPointer(vTextCoord, 2, GL_FLOAT, GL_FALSE, 0, (void*)((nt) * sizeof(textCoord)));
		glEnableVertexAttribArray(vTextCoord);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		this->clothProgram->useShader();

		this->clothProgram->setMat4("projection", proj);
		this->clothProgram->setMat4("modelView", view);
		this->clothProgram->setMat4("model", this->model);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBindVertexArray(this->objectVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBuffer);
		glDrawElements(GL_TRIANGLES, this->vertexIndices.size() * 3, GL_UNSIGNED_INT, (void*)0);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

private:
	float width, height, gridPoint, mass;
	float particleHeight, particleWidth, particleNum;
	float numVertices;

	GLuint objectVAO;
	GLuint vertexBuffer, normalBuffer, textcoordBuffer, indexBuffer;

	std::vector<ClothParticle> particle;
	Shaders* clothProgram;

	glm::mat4 model;
	std::vector<glm::vec3> vertexPositions;
	std::vector<glm::vec3> vertexNormals;
	std::vector<glm::vec2> vertexTextureCoords;
	std::vector<GLint> vertexIndices;

	std::vector<ClothSpring> springs;

	std::vector<ClothSpring> structural;
	std::vector<ClothSpring> shears;
	std::vector<ClothSpring> bend;

};


#endif // !CLOTH

