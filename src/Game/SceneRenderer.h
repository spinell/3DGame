#pragma once
#include "Mesh.h"

#include "vulkan/VulkanDescriptorPool.h"
#include "vulkan/vulkan.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <memory>

struct CTransform {
    glm::vec3 position = {0.f, 0.f, 0.f};
    glm::vec3 rotation = {0.f, 0.f, 0.f};
    glm::vec3 scale    = {1.f, 1.f, 1.f};
};
struct CMesh {
    Mesh mesh;
};
struct CMaterial {
    glm::vec4 ambient = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Texture   diffuseMap;
    Texture   normalMap;
    Texture   specularMap;
    glm::vec4 specular  = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float     shininess = 32;
    glm::vec2 texScale  = glm::vec2(1.0f, 1.0f);
    VkDescriptorSet descriptorSet0;
    VkDescriptorSet descriptorSet1;
};
struct CDirectionalLight {
    bool      enable = true;
    glm::vec3 color;
    glm::vec3 direction;
};
struct CPointLight {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float     constant;
    float     linear;
    float     quadratic;
    bool      enable = true;
};
struct CSpotLight {
    bool      enable = true;
    glm::vec3 color;
    glm::vec3 direction;
    float     range;
    float     cutOffAngle; // degrees
};

class SceneRenderer {
public:
    SceneRenderer();
    ~SceneRenderer();

    SceneRenderer(const SceneRenderer&)            = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;

    SceneRenderer(SceneRenderer&&)            = delete;
    SceneRenderer& operator=(SceneRenderer&&) = delete;

    void render(entt::registry*,
                VkCommandBuffer  cmd,
                const glm::mat4& proj,
                const glm::mat4& view,
                const glm::vec3& viewPosition);

    void setUseBlinnPhong(bool useBlinnPhong) { mUseBlinnPhong = useBlinnPhong;}
    bool isUseBlinnPhong(bool useBlinnPhong) const { return mUseBlinnPhong; }
    void toggleUseBlinnPhong() { mUseBlinnPhong = !mUseBlinnPhong; }

private:
    entt::registry* mRegistry{};
    bool                 mUseBlinnPhong = true;
    Buffer               mPerFrameBuffer;
    Buffer               mLightDataBuffer;
    std::shared_ptr<VulkanShaderProgram>  mMeshShader;
    GraphicPipeline      mMeshPipeline;
    VulkanDescriptorPool mDescriptorPool;
};
