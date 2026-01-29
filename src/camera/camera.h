#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Input {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	SPRINT,
	JOG,
	WALK,
	CRAWL
};

class Camera
{
public:

static constexpr float DEFAULT_YAW			= -90.0f;
static constexpr float DEFAULT_PITCH		= 0.0f;
static constexpr float DEFAULT_SPEED		= 1.0f;
static constexpr float DEFAULT_SENSITIVITY 	= 0.1f;
static constexpr float DEFAULT_ZOOM			= 45.0f;
static constexpr bool DEFAULT_SPRINTING		= false;
static constexpr bool DEFAULT_JOGGING		= false;
static constexpr bool DEFAULT_CRAWLING		= false;

    glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	bool Sprinting;
	bool Jogging;
	bool Crawling;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = DEFAULT_YAW, float pitch = DEFAULT_PITCH);

    const glm::mat4 GetViewMatrix() const;
    void ProcessKeyboard(Camera_Input input, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
       
	void UpdateCameraVectors();
private:

};

#endif