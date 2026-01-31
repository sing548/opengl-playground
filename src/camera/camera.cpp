#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(DEFAULT_SPEED),
      MouseSensitivity(DEFAULT_SENSITIVITY),
      Zoom(DEFAULT_ZOOM),
      Sprinting(DEFAULT_SPRINTING),
      Jogging(DEFAULT_JOGGING),
      Crawling(DEFAULT_CRAWLING)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    UpdateCameraVectors();
}

const glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Input input, float deltaTime)
{
	glm::vec3 oldPos = Position;
	if (input == SPRINT)
	{
		Sprinting = true;
		Jogging = false;
		Crawling = false;
	}
	if (input == JOG)
	{
		Sprinting = false;
		Jogging = true;
		Crawling = false;
	}
	if (input == WALK)
	{
		Sprinting = false;
		Jogging = false;
		Crawling = false;
	}
	if (input == CRAWL)
	{
		Sprinting = false;
		Jogging = false;
		Crawling = true;
	}

	float velocity;
	if (Sprinting)
		velocity = MovementSpeed * deltaTime * 50.0f;
	else if (Jogging)
		velocity = MovementSpeed * deltaTime * 10.0f;
	else if (Crawling)
		velocity = MovementSpeed * deltaTime * 0.1f;
	else
		velocity = MovementSpeed * deltaTime;
	if (input == FORWARD)
		Position += glm::normalize(glm::vec3(Front.x, 0.0f, Front.z)) * velocity;
	if (input == BACKWARD)
		Position -= glm::normalize(glm::vec3(Front.x, 0.0f, Front.z)) * velocity;
	if (input == LEFT)
		Position -= Right * velocity;
	if (input == RIGHT)
		Position += Right * velocity;
	
	Position.y = oldPos.y;
	if (input == UP)
		Position += WorldUp * velocity;
	if (input == DOWN)
		Position -= WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;
	Yaw   += xoffset;
	Pitch += yoffset;
	
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
	Zoom -= (float)yoffset;
	
	if (Zoom < 1.0f)
		Zoom = 1.0f;

	if (Zoom > 45.0f)
		Zoom = 45.0f;
}

void Camera::UpdateCameraVectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up	  = glm::normalize(glm::cross(Right, Front));
}
