#pragma once
#include <Engine/Event.h>
#include <glm/glm.hpp>

namespace Engine {

class CameraController {
public:
    CameraController();

    /// @brief
    /// @param fovy   Field of view in degree.
    /// @param width  Camera view width.
    /// @param height Camera view height.
    /// @param near   Camera near plane.
    /// @param far    Camera far plane.
    /// @param
    CameraController(float fovy, float width, float height, float near = 0.1f, float far = 1000.f);

    void onEvent(const Engine::Event& event);
    void onUpdate(float timeStep);

    [[nodiscard]] const glm::mat4& getProjectonMatrix() const { return mProjMatrix; }
    [[nodiscard]] const glm::mat4& getViewMatrix() const { return mViewMatrix; }
    [[nodiscard]] const glm::vec3& getPosition() const { return mPosition; }

private:
    float mNear{0.1f};
    float mFar{1000.f};
    float mFovy{45.0f};
    glm::mat4 mProjMatrix;
    glm::mat4 mViewMatrix       = glm::mat4(1);
    glm::vec3 mPosition         = {0.0f, 0.0f, 0.0f};
    glm::vec3 mForwardDirection = {0.0f, 0.0f, -1.0f}; // camera forward vector(look into the
                                                       // screen)
    glm::vec3 mRightDirection = {1.0f, 0.0f, 0.0f};    // camera right vector direction
    glm::vec3 mUpDirection    = {0.0f, 1.0f, 0.0f};    // camera up vector direction

    constexpr static float MIN_SPEED{0.0005f}, MAX_SPEED{2.0f};
};

} // namespace Engine
