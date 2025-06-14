#pragma once
#include <glm/glm.hpp>

namespace Engine {

    class Camera {
    public:
        Camera(float width, float height);
        Camera(float left, float right, float bottom, float top);

        void setProjection(float width, float height);
        void setProjection(float left, float right, float bottom, float top);
        void setPosition(const glm::vec3& position);
        void setRotation(float rotation);

        const glm::mat4& getProjectionMatrix() const { return mProjMatrix; }
        const glm::mat4& getViewMatrix() const { return mViewMatrix; }
        const glm::vec3& getPosition() const { return mPosition; }

    private:
        glm::mat4 mProjMatrix;
        glm::mat4 mViewMatrix = glm::mat4(1);
        glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
        float     mRotation{};
    };

} // namespace Engine
