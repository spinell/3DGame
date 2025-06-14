#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

Camera::Camera(float width, float height) {
    setProjection(width, height);

}

Camera::Camera(float left, float right, float bottom, float top) {
    setProjection(left, right, bottom, top);
}

void Camera::setProjection(float width, float height) {
    setProjection(-width/2, width/2, -height/2, height/2);
}

void Camera::setProjection(float left, float right, float bottom, float top) {
    mProjMatrix = glm::ortho(left, right, bottom, top, 1.0f, -1.0f);
}

void Camera::setPosition(const glm::vec3& position) {
    mPosition = position;

    const auto transform = glm::translate(glm::mat4(1), position);
    mViewMatrix = glm::inverse(transform);
}

}
