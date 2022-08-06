#include "Shader.h"
#include<glew.h>
#include<glfw3.h>

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	//创建File_buff
	std::ifstream vertexFile;
	std::ifstream fragmentFile;
	std::ifstream geometryFile;
	//创建string_buff
	std::stringstream vertexSStream;
	std::stringstream fragmentSStream;

	//异常抓取
	vertexFile.exceptions(std::ifstream::badbit || std::ifstream::failbit);
	fragmentFile.exceptions(std::ifstream::badbit || std::ifstream::failbit);
	geometryFile.exceptions(std::ifstream::badbit || std::ifstream::failbit);
	try
	{
		vertexFile.open(vertexPath);
		fragmentFile.open(fragmentPath);

		if (!vertexFile.is_open() || !fragmentFile.is_open())
		{
			throw std::exception("open file error");
		}
		//将资料从硬盘读到数据流中
		vertexSStream << vertexFile.rdbuf();
		fragmentSStream << fragmentFile.rdbuf();
		//关闭文件
		vertexFile.close();
		fragmentFile.close();
		//数据流到string
		vertexString = vertexSStream.str();
		fragmentString = fragmentSStream.str();
		if (geometryPath != nullptr)
		{
			geometryFile.open(geometryPath);
			std::stringstream geometrySStream;
			geometrySStream << geometryFile.rdbuf();
			geometryFile.close();
			geometryString = geometrySStream.str();
		}

		vertexSource = vertexString.c_str();
		fragmentSource = fragmentString.c_str();
	}
	catch (std::ifstream::failure& ex)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << ex.what() << std::endl;
	}

	//compile shaders
	unsigned int vertex, fragment;
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSource, nullptr);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSource, nullptr);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");

	// if geometry shader is given, compile geometry shader
	unsigned int geometry;
	if (geometryPath != nullptr)
	{
		geometrySource = geometryString.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &geometrySource, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	if (geometryPath != nullptr)
		glAttachShader(ID, geometry);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	std::cout << "vertex加载成功" << vertexPath << std::endl;
	std::cout << "fragment加载成功" << fragmentPath << std::endl;

	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
		glDeleteShader(geometry);
}
Shader::~Shader() {}

void Shader::use()
{
	glUseProgram(ID);
}

void Shader::setVec2(const std::string& paramNameString, glm::vec2 param)
{
	glUniform2f(glGetUniformLocation(ID, paramNameString.c_str()), param.x, param.y);
}
void Shader::setVec3(const std::string& paramNameString, glm::vec3 param)
{
	glUniform3f(glGetUniformLocation(ID, paramNameString.c_str()), param.x, param.y, param.z);
}
void Shader::setFloat(const std::string& paramNameString, float param)
{
	glUniform1f(glGetUniformLocation(ID, paramNameString.c_str()), param);
}
void Shader::setInt(const std::string& paramNameString, int slot)
{
	glUniform1i(glGetUniformLocation(ID, paramNameString.c_str()), slot);
}
void Shader::setMat4(const std::string& paramNameString, glm::mat4 param)
{
	glUniformMatrix4fv(glGetUniformLocation(ID, paramNameString.c_str()), 1, GL_FALSE, glm::value_ptr(param));
}
//错误抓取
void Shader::checkCompileErrors(unsigned int ID, std::string type)
{
	int success;
	char infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(ID, 1024, nullptr, infoLog);
			std::cout << "shader compile error:" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ID, 1024, nullptr, infoLog);
			std::cout << "program link error:" << infoLog << std::endl;
		}
	}
}