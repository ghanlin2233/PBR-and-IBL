#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "Shader.h"
#include "Mesh.h"
#include "StringUtil.h"

class Model
{
public:
	Model(std::string const& path);
	void Draw(std::shared_ptr<Shader> shader);

	static GLuint LoadTexture(const GLchar* path, GLint wrapMode = GL_REPEAT, GLint MagFilterMode = GL_LINEAR, GLint MinFilterMode = GL_LINEAR_MIPMAP_LINEAR);
	static GLuint LoadTextureFromAssImp(const aiTexture* aiTex, GLint wrapMode = GL_REPEAT, GLint MagFilterMode = GL_LINEAR, GLint MinFilterMode = GL_LINEAR_MIPMAP_LINEAR);
private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::map<std::string, Texture> textures_loaded;
	void loadModel(std::string const& path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene);
};

