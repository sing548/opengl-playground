#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



class Camera
{
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 front = (glm::vec3(1.0f, 0.0f, 0.0f)));

	const glm::mat4 GetViewMatrix() const;
	void ProcessMouseScroll(float yoffset);

	void SetPosition(glm::vec3 pos) { position_ = pos; };
	void SetBasis(glm::vec3 pos, glm::vec3 front, glm::vec3 up);

	float GetZoom() const { return zoom_; };
	const glm::vec3 GetPosition() const { return position_; };
	const glm::vec3 GetWorldUp() const { return worldUp_; };
private:
	static constexpr float DEFAULT_ZOOM = 45.0f;

	float zoom_;

    glm::vec3 position_;
	glm::vec3 front_;
	glm::vec3 up_;
	glm::vec3 right_;
	glm::vec3 worldUp_;
};

#endif