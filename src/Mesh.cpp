#include "Mesh.h"

const char* DIFFUSE_TEX_NAME = "texture_diffuse";
const char* SPECULAR_TEX_NAME = "texture_specular";
const char* NORMAL_TEX_NAME = "texture_normal";
const char* HEIGHT_TEX_NAME = "texture_height";


Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	setupMesh();
}

void Mesh::Draw(std::shared_ptr<Shader> shader)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
			// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].type;
		if (name == DIFFUSE_TEX_NAME)
			number = std::to_string(diffuseNr++);
		else if (name == SPECULAR_TEX_NAME)
			number = std::to_string(specularNr++); // transfer unsigned int to string
		else if (name == NORMAL_TEX_NAME)
			number = std::to_string(normalNr++); // transfer unsigned int to string
		else if (name == HEIGHT_TEX_NAME)
			number = std::to_string(heightNr++); // transfer unsigned int to string

		// now set the sampler to the correct texture unit
		//glUniform1i(glGetUniformLocation(shader., (name + number).c_str()), i);
		//std::cout << (name + number).c_str() << std::endl;
		//std::cout << textures.size() << std::endl;
		shader->setInt((name + number).c_str(), i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(GL_FLOAT)));

	glBindVertexArray(0);
}