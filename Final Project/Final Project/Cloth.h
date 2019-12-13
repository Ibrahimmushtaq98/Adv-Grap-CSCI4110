#pragma once

#ifndef CLOTH
#define CLOTH

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "ClothParticle.h"
#include "ClothSpringV2.h"
#include "ShadersV2.h"
#include "texture.h"

class Cloth {
	public:
		Cloth(float w, float h, float n, float m);
		void initGeometryParticle(float h, float w);
		void renderGeometry(glm::mat4 proj, glm::mat4 view);
		void updateGeometry(float t);
		void createGeometry();

	private:
		float width, height, numParticleHeight, numParticleWidth, numParticle, numVerticies;
		float gridPoint, mass;

		GLuint objectVAO;
		GLuint vBuffer, nBuffer, cBuffer, tbuffer, ibuffer;
		GLuint tBuffer, textBuffer, textID;
		Shaders* ClothShader;
		glm::mat4 model;

		std::vector<glm::vec3> vertexPositions;
		std::vector<glm::vec3> vertexNormals;
		std::vector<glm::vec3> vertexColors;
		std::vector<glm::vec2> vertexTextureCoords;
		std::vector<GLint> vertexIndices;

		std::vector<GLint> index;

		std::vector<ClothParticle> particles;
		std::vector<ClothSpring> springs;

		std::vector<ClothSpring> structural;
		std::vector<ClothSpring> shears;
		std::vector<ClothSpring> bend;

		bool use = true;
};

#endif // !CLOTH

