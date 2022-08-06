#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
class Camera
{
public:
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Forward(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updataCameraPos();
	}
	Camera(glm::vec3 position, float pitch, float yaw, glm::vec3 worldup);
	glm::mat4 getViewMatrix();
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	void processMouseMovement(float deltaX, float deltaY);
	void ProcessMouseScroll(float yoffset);
	glm::vec3 Position;
	glm::vec3 Forward;
	glm::vec3 Right;
	glm::vec3 Up;
	glm::vec3 WorldUp;

	float Pitch, Yaw;
	float MovementSpeed = 3;
	float MouseSensitivity = 0.01;
	float zoom;
private:
	void updataCameraPos();

};

