#pragma once
#include <glew.h>
#include <glfw3.h>

#include <vector>
#include <memory>

#include "Shader.h"
class Scenes
{
public:

	virtual void renderScene() = 0;
};

class Cube :public Scenes
{
public:
	Cube(std::shared_ptr<Shader> _ptr, glm::mat4 model) :
		ptr(_ptr), modelMatrix(model)
	{};
	void renderScene();

	glm::mat4 modelMatrix;
	std::shared_ptr<Shader> ptr;
	unsigned int VAO = 0, VBO = 0;
};

class Plane :public Scenes
{
public:
	Plane(std::shared_ptr<Shader> _ptr, glm::mat4 model) :
		ptr(_ptr), modelMatrix(model)
	{};
	void renderScene();

	glm::mat4 modelMatrix;
	std::shared_ptr<Shader> ptr;
	unsigned int VAO = 0, VBO = 0;
};
class light :public Scenes
{
public:
	light(std::shared_ptr<Shader> _ptr, glm::mat4 model) :
		ptr(_ptr), modelMatrix(model)
	{};
	void renderScene();

	glm::mat4 modelMatrix;
	std::shared_ptr<Shader> ptr;
	unsigned int VAO = 0, VBO = 0;
};
class quadVAO :public Scenes
{
public:
	quadVAO(std::shared_ptr<Shader> _ptr, glm::mat4 model) :
		ptr(_ptr), modelMatrix(model)
	{};
	void renderScene();

	glm::mat4 modelMatrix;
	std::shared_ptr<Shader> ptr;
	unsigned int VAO = 0, VBO = 0;
};

