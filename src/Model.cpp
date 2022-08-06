#include "Model.h"
#include "stb_image.h"
Model::Model(std::string const& path)
{
	loadModel(path);
}

void Model::Draw(std::shared_ptr<Shader> shader)
{
	for (auto& mesh : meshes)
	{
		mesh.Draw(shader);
	}
}

void Model::loadModel(std::string const& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
	//std::unique_ptr<const aiScene> ptr(scene);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	std::cout << "success!" << directory << std::endl;
	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* curMesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(curMesh, scene));
		//std::cout << "curMesh = " << &curMesh << scene<< std::endl;
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
		//std::cout << "node - > " << node->mChildren[i]<< scene << std::endl;
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
	std::vector<Texture> Textures;

	//Position
	for (auto i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vec(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.Position = vec;

		vec.x = mesh->mNormals[i].x;
		vec.y = mesh->mNormals[i].y;
		vec.z = mesh->mNormals[i].z;
		vertex.Normal = vec;

		if (mesh->mTextureCoords[0])
		{
			vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else
		{
			vertex.TexCoords = glm::vec2(0);
		}

		Vertices.push_back(vertex);
	}

	//Indices
	for (auto i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (auto j = 0; j < face.mNumIndices; j++)
			Indices.push_back(face.mIndices[j]);
	}

	//Material
	std::cout << mesh->mMaterialIndex << std::endl;
	if (mesh->mMaterialIndex >= 0)
	{
		auto material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material,
			aiTextureType_DIFFUSE, "texture_diffuse", scene);
		Textures.insert(Textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = LoadMaterialTextures(material,
			aiTextureType_SPECULAR, "texture_specular", scene);
		Textures.insert(Textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<Texture> emissiveMaps = LoadMaterialTextures(material,
			aiTextureType_EMISSIVE, "texture_emissive", scene);
		Textures.insert(Textures.end(), emissiveMaps.begin(), emissiveMaps.end());

		std::vector<Texture> normalMaps = LoadMaterialTextures(material,
			aiTextureType_NORMALS, "texture_normal", scene);
		Textures.insert(Textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> shininessMaps = LoadMaterialTextures(material,
			aiTextureType_SHININESS, "texture_shininess", scene);
		Textures.insert(Textures.end(), shininessMaps.begin(), shininessMaps.end());

	}

	return Mesh(Vertices, Indices, Textures);
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene)
{
	std::vector<Texture> textures;
	for (auto i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		auto toFind = textures_loaded.find(str.C_Str());
		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		if (toFind != textures_loaded.end())
		{
			textures.push_back(toFind->second);
		}
		else
		{
			Texture tex;
			auto filePath = StringUtil::Format("%s/%s", directory.c_str(), str.C_Str());

			auto aitexture = scene->GetEmbeddedTexture(str.C_Str());
			if (aitexture != nullptr) {

				std::cout << "正在导入内嵌材质" << std::endl;
				tex.id = LoadTextureFromAssImp(aitexture, GL_CLAMP, GL_LINEAR, GL_LINEAR);
			}
			else {
				std::cout << "正在导入材质" << std::endl;
				tex.id = LoadTexture(filePath.c_str(), GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
			}
			tex.type = typeName;
			std::cout << typeName << std::endl;
			tex.path = str;
			textures.push_back(tex);
			textures_loaded[str.C_Str()] = tex;
		}
	}
	return textures;
}

GLuint Model::LoadTexture(const GLchar* path, GLint wrapMode, GLint MagFilterMode, GLint MinFilterMode)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//float borderColor[] = { 1.0f, 0.6f, 0.6f, 1.0f };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilterMode);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

GLuint Model::LoadTextureFromAssImp(const aiTexture* aiTex, GLint wrapMode, GLint MagFilterMode, GLint MinFilterMode)
{
	if (aiTex == nullptr)
		return 0;
	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilterMode);


	int width, height, nrChannels;
	unsigned char* image_data = nullptr;
	if (aiTex->mHeight == 0)
	{
		image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTex->pcData), aiTex->mWidth, &width, &height, &nrChannels, 0);
	}
	else
	{
		image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTex->pcData), aiTex->mWidth * aiTex->mHeight, &width, &height, &nrChannels, 0);
	}

	if (image_data != nullptr)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
	}

	glGenerateMipmap(GL_TEXTURE_2D);
	return textureID;
}

