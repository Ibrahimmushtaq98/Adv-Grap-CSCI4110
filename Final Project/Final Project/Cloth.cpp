#include "Cloth.h"
#include "ClothParticle.h"
#include "ClothSpringV2.h"
#include <vector>
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include "texture.h"
#include "ShadersV2.h"

Cloth::Cloth(float w, float h, float n, float m) {
	int vPosition, vNormal, vTextCoord;
	this->width = w;
	this->height = h;
	this->gridPoint = n;
	this->mass = m;

	this->numParticleHeight = this->width * this->gridPoint;
	this->numParticleWidth = this->height * this->gridPoint;
	this->numParticle = this->numParticleHeight * this->numParticleWidth;
	this->numVerticies = (this->numParticleWidth - 1) * (this->numParticleHeight - 1) * 6;

	this->vertexPositions = std::vector<glm::vec3>(numParticle);
	this->vertexNormals = std::vector<glm::vec3>(numParticle);
	this->vertexTextureCoords = std::vector<glm::vec2>(numParticle);
	this->vertexIndices = std::vector<GLint>(numVerticies);

	this->particles = std::vector<ClothParticle>(numParticle);

	this->model = glm::mat4(1.0f);

	initGeometryParticle(numParticleHeight, numParticleWidth);

	int indexK = 0;
	int i, j,k = 0;

	for (i = 0; i < this->numParticleHeight - 1; i++){
		for (j = 0; j < this->numParticleWidth; j++){
			this->vertexIndices[k] = i * this->numParticleWidth + j;
			this->vertexIndices[k + 1] = (i + 1) * this->numParticleHeight + j;
			k += 2;
		}
	}

	glGenVertexArrays(1, &this->objectVAO);
	glBindVertexArray(this->objectVAO);

	int nv, nn, nt, ni;
	GLfloat* vertices;
	GLfloat* normals;
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

	nn = this->vertexNormals.size();
	normals = new GLfloat[nn];
	count = 0;
	for (i = 0; i < nn; i++) {
		normals[i++] = this->vertexNormals[count].x;
		normals[i++] = this->vertexNormals[count].y;
		normals[i++] = this->vertexNormals[count].z;
	}

	ni = this->vertexIndices.size();
	indexs = new GLuint[ni];
	for (int i = 0; i < ni; i++) {
		indexs[i] = this->vertexIndices[i];
	}

	ClothShader = new Shaders((char*)"Shader/clothShader.vs", (char*)"Shader/clothShader.fs");
	
	glGenBuffers(1, &this->vBuffer);
	std::cout << &this->vBuffer << std::endl;
	glBindBuffer(GL_ARRAY_BUFFER, this->vBuffer);
	glBufferData(GL_ARRAY_BUFFER, (nv) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, nv * sizeof(GLfloat), vertices);
	//glBufferSubData(GL_ARRAY_BUFFER, nv * sizeof(GLfloat), nn * sizeof(GLfloat), normals);

	glGenBuffers(1, &this->ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni * sizeof(GLuint), indexs, GL_STATIC_DRAW);

	ClothShader->useShader();
	glBindBuffer(GL_ARRAY_BUFFER, this->vBuffer);
	vPosition = glGetAttribLocation(ClothShader->getShaderID(), "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	//glGenBuffers(1, &this->vBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, this->vBuffer);
	//glBufferData(GL_ARRAY_BUFFER, 3 * this->vertexPositions.size() * sizeof(GLfloat), &this->vertexPositions[0].x, GL_STATIC_DRAW);

	//glGenBuffers(1, &this->nBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, this->nBuffer);
	//glBufferData(GL_ARRAY_BUFFER, 3 * this->vertexNormals.size() * sizeof(GLfloat), &this->vertexNormals[0].x, GL_STATIC_DRAW);
	////glEnableVertexAttribArray(1);
	////glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	//glGenBuffers(1, &this->tbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, this->tbuffer);
	//glBufferData(GL_ARRAY_BUFFER, 2 * this->vertexTextureCoords.size() * sizeof(GLfloat), &this->vertexTextureCoords[0].x, GL_STATIC_DRAW);

	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	//Texture* texture = loadTexture("Texture/texture.jpg");
	//glGenTextures(1, &this->tBuffer);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, tBuffer);
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture->width, texture->height);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height, GL_RGB, GL_UNSIGNED_BYTE, texture->data);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Cloth::initGeometryParticle(float h, float w) {
	int currentParticle = 0;
	int i, j;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			int subscripts = i * w + 3 * j;

			ClothParticle part = ClothParticle();
			part.setPosition(glm::vec3(j / this->gridPoint, 0.0f, i / this->gridPoint));
			part.setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
			part.setMass(this->mass);

			//Fixed Point
			if ((i == this->gridPoint - 1) || (i == this->gridPoint - 1 && j == this->gridPoint - 1)) {
				part.setFixed(true);
			}
			else {
				part.setFixed(false);
			}

			part.applyTransformations();
			this->particles[currentParticle] = part;
			currentParticle++;
		}
	}

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			int current = i * w + j;

			ClothParticle part = this->particles[current];

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
				ClothParticle tmpP = this->particles[right];
				ClothSpring coil = ClothSpring();
				coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
				coil.setEndParticles(&part, &tmpP);
				coil.setSpringConstant(structSpringConst);
				coil.setDampeningConstant(structDampConst);
				this->structural.push_back(coil);
			}
			if (i < h - 1) {
				ClothParticle tmpP = this->particles[bottom];
				ClothSpring coil = ClothSpring();
				coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
				coil.setEndParticles(&part, &tmpP);
				coil.setSpringConstant(structSpringConst);
				coil.setDampeningConstant(structDampConst);
				this->structural.push_back(coil);
			}

			//Setup the shear spring

			if (i > 0 && j < w - 1) {
				ClothParticle tmpP = this->particles[topRight];
				ClothSpring coil = ClothSpring();
				coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
				coil.setEndParticles(&part, &tmpP);
				coil.setSpringConstant(shearSpringConst);
				coil.setDampeningConstant(shearDampConst);
				this->shears.push_back(coil);
			}
			if (i < h - 1 && j < w - 1) {
				ClothParticle tmpP = this->particles[bottomRight];
				ClothSpring coil = ClothSpring();
				coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
				coil.setEndParticles(&part, &tmpP);
				coil.setSpringConstant(shearSpringConst);
				coil.setDampeningConstant(shearDampConst);
				this->shears.push_back(coil);
			}

			//Setup the bend

			if (j < w - 2) {
				ClothParticle tmpP = this->particles[rightRight];
				ClothSpring coil = ClothSpring();
				coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
				coil.setEndParticles(&part, &tmpP);
				coil.setSpringConstant(bendSpringConst);
				coil.setDampeningConstant(bendDampConst);
				this->bend.push_back(coil);
			}
			if (i < h - 2) {
				ClothParticle tmpP = this->particles[bottomBottom];
				ClothSpring coil = ClothSpring();
				coil.setRestLength(glm::length(part.getPosition() - tmpP.getPosition()));
				coil.setEndParticles(&part, &tmpP);
				coil.setSpringConstant(bendSpringConst);
				coil.setDampeningConstant(bendDampConst);
				this->bend.push_back(coil);
			}
		}
	}
}

void Cloth::renderGeometry(glm::mat4 proj, glm::mat4 view){
	//int i,j;
	//int vPosition, vNormal, vTextCoord;

	//for (i = 0; i <this->numParticleHeight ; ++i){
	//	for (j = 0; j < this->numParticleWidth; ++j)
	//	{
	//		int current = i * this->numParticleHeight + j;
	//		this->vertexPositions[current] = this->particles[current].getPosition();
	//		this->vertexTextureCoords[current] = glm::vec2((float)j / (this->numParticleWidth - 1), (float)i / (this->numParticleHeight - 1));
	//	}
	//}

	//int index = 0;
	//for (i = 0; i < this->numParticleHeight - 1; ++i){
	//	for (int j = 0; j < this->numParticleWidth - 1; ++j){
	//		glm::vec3 normal;

	//		int current = i * this->numParticleWidth + j;
	//		int right = current + 1;
	//		int bottom = current + this->numParticleWidth;
	//		 int bottomRight = bottom + 1;

	//		this->vertexIndices[index] = current;
	//		this->vertexIndices[index + 1] = bottom;
	//		this->vertexIndices[index + 2] = bottomRight;

	//		normal = glm::normalize(
	//			glm::cross(this->vertexPositions[bottomRight] - this->vertexPositions[current], this->vertexPositions[bottom] - this->vertexPositions[current])
	//		);
	//		this->vertexNormals[current] += normal;
	//		this->vertexNormals[bottom] += normal;
	//		this->vertexNormals[bottomRight] += normal;


	//		this->vertexIndices[index + 3] = current;
	//		this->vertexIndices[index + 4] = bottomRight;
	//		this->vertexIndices[index + 5] = right;

	//		normal = glm::normalize(
	//			glm::cross(this->vertexPositions[right] - this->vertexPositions[current], this->vertexPositions[bottomRight] - this->vertexPositions[current])
	//		);
	//		this->vertexNormals[current] += normal;
	//		this->vertexNormals[bottomRight] += normal;
	//		this->vertexNormals[right] += normal;

	//		index += 6;
	//	}
	//}

	//for (int i = 0; i < this->numParticle; ++i){
	//	this->vertexNormals[i] = normalize(this->vertexNormals[i]);
	//}

	if (use) {
		std::cout << "Vertex Normal Size: " << this->vertexNormals.size() << "\n";
		std::cout << "Vertex Indicies Size: " << this->vertexIndices.size() << "\n";
		std::cout << "Vertex Positon Size: " << this->vertexPositions.size() << "\n";
		std::cout << "Vertex Textcoord Size: " << this->vertexTextureCoords.size() << "\n";
		//for (int i = 0; i < this->vertexPositions.size(); i++) {
		//	std::cout << glm::to_string(this->vertexPositions[i]) << " ";
		//}
		use = false;
	}

	//glGenVertexArrays(1, &this->objectVAO);
	//glBindVertexArray(this->objectVAO);

	//int nv, nn, nt, ni;
	//GLfloat* vertices;
	//GLfloat* normals;
	//GLuint* indexs;
	//int count = 0;

	//nv = this->vertexPositions.size() * 3;
	//vertices = new GLfloat[nv];
	//for (i = 0; i < nv;) {
	//	vertices[i++] = this->vertexPositions[count].x;
	//	vertices[i++] = this->vertexPositions[count].y;
	//	vertices[i++] = this->vertexPositions[count].z;
	//	count++;
	//}

	//nn = this->vertexNormals.size();
	//normals = new GLfloat[nn];
	//count = 0;
	//for (i = 0; i < nn; i++) {
	//	normals[i++] = this->vertexNormals[count].x;
	//	normals[i++] = this->vertexNormals[count].y;
	//	normals[i++] = this->vertexNormals[count].z;
	//}

	//ni = this->vertexIndices.size();
	//indexs = new GLuint[ni];
	//for (int i = 0; i < ni; i++) {
	//	indexs[i] = this->vertexIndices[i];
	//}


	//glGenBuffers(1, &this->vBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, this->vBuffer);
	//glBufferData(GL_ARRAY_BUFFER, (nv) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	////glBufferSubData(GL_ARRAY_BUFFER, 0, nv * sizeof(GLfloat), vertices);
	////glBufferSubData(GL_ARRAY_BUFFER, nv * sizeof(GLfloat), nn * sizeof(GLfloat), normals);

	//glGenBuffers(1, &this->ibuffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibuffer);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni * sizeof(GLuint), indexs, GL_STATIC_DRAW);

	//ClothShader->useShader();
	//glBindBuffer(GL_ARRAY_BUFFER, this->vBuffer);
	//vPosition = glGetAttribLocation(ClothShader->getShaderID(), "vPosition");
	//glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(vPosition);
	//vNormal = glGetAttribLocation(ClothShader->getShaderID(), "vNormal");
	//glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)((nv) * sizeof(vertices)));
	//glEnableVertexAttribArray(vNormal);

	{
		//glGenBuffers(1, &this->vBuffer);
		//glBindBuffer(GL_ARRAY_BUFFER, this->vBuffer);
		//glBufferData(GL_ARRAY_BUFFER, this->vertexPositions.size() * sizeof(glm::vec3), &this->vertexPositions[0].x, GL_STATIC_DRAW);
		////glEnableVertexAttribArray(0);
		////glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

		//glGenBuffers(1, &this->nBuffer);
		//glBindBuffer(GL_ARRAY_BUFFER, this->nBuffer);
		//glBufferData(GL_ARRAY_BUFFER, this->vertexNormals.size() * sizeof(glm::vec3), &this->vertexNormals[0].x, GL_STATIC_DRAW);
		////glEnableVertexAttribArray(1);
		////glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

		//glGenBuffers(1, &this->tbuffer);
		//glBindBuffer(GL_ARRAY_BUFFER, this->tbuffer);
		//glBufferData(GL_ARRAY_BUFFER, this->vertexTextureCoords.size() * sizeof(glm::vec2), &this->vertexTextureCoords[0].x, GL_STATIC_DRAW);
		////glEnableVertexAttribArray(3);
		////glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		//
		//ClothShader->useShader();
		//vPosition = glGetAttribLocation(ClothShader->getShaderID(), "vPosition");
		//glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		//glEnableVertexAttribArray(vPosition);
		//vNormal = glGetAttribLocation(ClothShader->getShaderID(), "vNormal");
		//glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		//glEnableVertexAttribArray(vNormal);
		//vTextCoord = glGetAttribLocation(ClothShader->getShaderID(), "vTextCoord");
		//glVertexAttribPointer(vTextCoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		//glEnableVertexAttribArray(vTextCoord);
	}
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	ClothShader->useShader();

	ClothShader->setMat4("projection", proj);
	ClothShader->setMat4("modelView", view);
	ClothShader->setMat4("model", this->model);

	glBindVertexArray(this->objectVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibuffer);
	glDrawElements(GL_TRIANGLES, this->vertexIndices.size() * 3, GL_UNSIGNED_INT, (void*)0);
}

void Cloth::updateGeometry(float deltaTime){
	//For each coil in the cloth, update its geometry
	//which consists of computing the forces acting
	//on the cloth particles attached to both ends
	//of the coil

	float newTime = deltaTime / 10.0f;

	for (int i = 0; i < 10; ++i){
		for (auto& coil : this->structural){
			coil.updateGeometry(deltaTime);
		}
		for (auto& coil : this->shears){
			coil.updateGeometry(deltaTime);
		}
		for (auto& coil : this->bend){
			coil.updateGeometry(deltaTime);
		}

		//Now update the positions and velocities
		//of the cloth particles
		for (auto& particle : this->particles){
			particle.updateGeometry(deltaTime);
		}

		for (int i = 0; i < 4; ++i){
			for (auto& coil : this->structural){
				coil.checkStretch();
			}
			for (auto& coil : this->shears){
				coil.checkStretch();
			}
		}
	}
}