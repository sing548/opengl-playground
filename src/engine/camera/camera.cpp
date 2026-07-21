#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, glm::vec3 front)
    : position_(position), worldUp_(up), front_(front), zoom_(DEFAULT_ZOOM)
{
	SetBasis(position, front, worldUp_);
}

const glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::ProcessMouseScroll(float yoffset)
{
	zoom_ -= (float)yoffset;
	
	if (zoom_ < 1.0f)
		zoom_ = 1.0f;

	if (zoom_ > 45.0f)
		zoom_ = 45.0f;
}

void Camera::SetBasis(glm::vec3 pos, glm::vec3 front, glm::vec3 up)
{
	position_ = pos;
	front_ = glm::normalize(front);
	right_ = glm::normalize(glm::cross(front_, up));
	up_ = glm::cross(right_, front_);
}
