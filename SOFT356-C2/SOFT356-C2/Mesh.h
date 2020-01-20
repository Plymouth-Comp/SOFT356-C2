#pragma once
#include <vector>
#include <string>

#include "Shader.h"

//OpenGL
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GLFW/glfw3.h"

//GLM
#include <glm.hpp> //includes GML
#include <ext/matrix_transform.hpp> // GLM: translate, rotate
#include <ext/matrix_clip_space.hpp> // GLM: perspective and ortho 
#include <gtc/type_ptr.hpp> // GLM: access to the value_ptr

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};


class Mesh {
public:
	/*  Mesh Data  */
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	/*  Functions  */
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(Shader shader);

	/*  Render data  */
	GLuint VAO, VBO, EBO;
	/*  Functions    */
	void setupMesh();
};

