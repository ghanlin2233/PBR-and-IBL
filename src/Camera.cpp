#include "Camera.h"

Camera::Camera(glm::vec3 cameraPos, float pitch, float yaw, glm::vec3 worldup)
{
	this->Position = cameraPos;
	this->WorldUp = worldup;
	this->Pitch = pitch;
	this->Yaw = yaw;
	updataCameraPos();
}
glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(Position, Position + Forward, WorldUp);
}
void Camera::updataCameraPos()
{
	glm::vec3 front;
	front.x = glm::cos(glm::radians(Pitch)) * glm::cos(glm::radians(Yaw));
	front.y = glm::sin(glm::radians(Pitch));
	front.z = glm::cos(glm::radians(Pitch)) * glm::sin(glm::radians(Yaw));
	this->Forward = glm::normalize(front);
	this->Right = glm::normalize(glm::cross(WorldUp, Forward));
	this->Up = glm::normalize(glm::cross(Forward, Right));
}
void Camera::processMouseMovement(float deltaX, float deltaY)
{
	Yaw += deltaX * MouseSensitivity;
	Pitch += deltaY * MouseSensitivity;
	bool constrainPitch = true;

	updataCameraPos();
}
void Camera::ProcessMouseScroll(float yoffset)
{
	this->zoom -= (float)yoffset;
	/*if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 45.0f)
		zoom = 45.0f;*/
}
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = MovementSpeed * deltaTime;
	if (direction == FORWARD)
		this->Position += this->Forward * velocity;
	//std::cout << this->Position.x << std::endl;
	if (direction == BACKWARD)
		this->Position -= this->Forward * velocity;
	if (direction == LEFT)
		this->Position += this->Right * velocity;
	if (direction == RIGHT)
		this->Position -= this->Right * velocity;
	if (direction == UP)
		this->Position += this->Up * velocity;
	if (direction == DOWN)
		this->Position -= this->Up * velocity;
}