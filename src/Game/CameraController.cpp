#pragma once
#include "CameraController.h"

#include <Engine/Event.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

CameraController::CameraController() {
    mProjMatrix = glm::perspectiveFov(glm::radians(mFovy), 800.f, 600.f, mNear, mFar);
}

CameraController::CameraController(float fovy, float width, float height, float near, float far)
 : mFovy(fovy)
 , mNear(near)
 , mFar(far)
 {
    mProjMatrix = glm::perspectiveFov(glm::radians(mFovy), 800.f, 600.f, mNear, mFar);
}

void CameraController::onEvent(const Engine::Event& event) {
    event.dispatch<Engine::WindowResizedEvent>([this](const auto& e) {
        mProjMatrix = glm::perspectiveFov(mFovy, (float)e.GetWidth(), (float)e.GetHeight(), mNear, mFar);
    });
}

void CameraController::onUpdate(float timeStep) {
    if (Engine::Input::IsButtonDown(Engine::MouseButton::Right)) {
        const glm::vec2& mouseDelta{Input::GetMousePositionDelta().first,
                                    Input::GetMousePositionDelta().second};

        const float xAngle = mouseDelta.x * glm::radians(1.0f);
        const float yAngle = mouseDelta.y * glm::radians(1.0f);

        if (yAngle != 0) {
            auto yRotation    = glm::rotate(glm::mat4(1.0f), -yAngle, mRightDirection);
            mForwardDirection = yRotation * glm::vec4(mForwardDirection, 1.0f);
            mUpDirection      = yRotation * glm::vec4(mUpDirection, 1.0f);
        }
        if (xAngle != 0) {
            auto yRotation    = glm::rotate(glm::mat4(1.0f), -xAngle, {0.f, 1.f, 0.f});
            mForwardDirection = yRotation * glm::vec4(mForwardDirection, 1.0f);
            mRightDirection   = yRotation * glm::vec4(mRightDirection, 1.0f);
        }
    }

    // Update position
    if (Engine::Input::IsKeyDown(Engine::KeyCode::Q)) {
        mPosition += mUpDirection * timeStep * MAX_SPEED;
    }
    if (Engine::Input::IsKeyDown(Engine::KeyCode::E)) {
        mPosition -= mUpDirection * timeStep * MAX_SPEED;
    }
    if (Engine::Input::IsKeyDown(Engine::KeyCode::W)) {
        mPosition += mForwardDirection * timeStep * MAX_SPEED;
    }
    if (Engine::Input::IsKeyDown(Engine::KeyCode::S)) {
        mPosition -= mForwardDirection * timeStep * MAX_SPEED;
    }
    if (Engine::Input::IsKeyDown(Engine::KeyCode::A)) {
        mPosition -= mRightDirection * timeStep * MAX_SPEED;
    }
    if (Engine::Input::IsKeyDown(Engine::KeyCode::D)) {
        mPosition += mRightDirection * timeStep * MAX_SPEED;
    }

    mViewMatrix = glm::lookAt(mPosition, mPosition + mForwardDirection, glm::vec3{0.f, 1.0f, 0.f});
}

} // namespace Engine
