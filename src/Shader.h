#pragma once

#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include<iostream>
#include<string>
#include <sstream>
#include <fstream>

class Shader
{
public:
	Shader() {};
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	~Shader();
	std::string vertexString;
	std::string fragmentString;
	std::string geometryString;
	const char* vertexSource;
	const char* fragmentSource;
	const char* geometrySource;
	unsigned int ID;//shader program ID
	void use();

	//uniform¹¤¾ßº¯Êý
	void setVec2(const std::string& paramNameString, glm::vec2 param);
	void setVec3(const std::string& paramNameString, glm::vec3 param);
	void setFloat(const std::string& paramNameString, float param);
	void setInt(const std::string& paramNameString, int slot);
	void setMat4(const std::string& paramNameString, glm::mat4 param);

private:
	void checkCompileErrors(unsigned int ID, std::string type);
};