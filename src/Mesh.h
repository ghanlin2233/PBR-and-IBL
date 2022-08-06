#pragma once
#include <glm.hpp>
#include <string>
#include <vector>
#include "Shader.h"
#include <glew.h>
#include <assimp/types.h>
struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tanget;
	glm::vec3 Bitangent;
};

struct Texture
{
	unsigned int id;
	std::string type;
	aiString path;
};
class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Mesh(float vertices_[]);
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(std::shared_ptr<Shader> shader);
private:
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};