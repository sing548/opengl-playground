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

static constexpr float YAW			= -90.0f;
static constexpr float PITCH		= 0.0f;
static constexpr float SPEED		= 0.05f;
static constexpr float SENSITIVITY 	= 0.1f;
static constexpr float ZOOM			= 45.0f;
static constexpr bool SPRINTING		= false;
static constexpr bool JOGGING		= false;
static constexpr bool CRAWLING		= false;

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
           float yaw = YAW, float pitch = PITCH);

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(Camera_Input input, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
       
private:

    void updateCameraVectors();
};

#endif