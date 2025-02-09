#pragma once
#include "Mesh.h"

#include "vulkan/VulkanDescriptorPool.h"
#include "vulkan/vulkan.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

struct CTransform {
    glm::vec3 position = {0.f, 0.f, 0.f};
    glm::vec3 rotation = {0.f, 0.f, 0.f};
    glm::vec3 scale    = {1.f, 1.f, 1.f};
};
struct CMesh {
    Mesh mesh;
};
struct CMaterial {
    glm::vec4 ambient   = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 diffuse   = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 specular  = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float     shininess = 32;
};

struct CPointLight {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
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
                Texture          texture,
                const glm::mat4& proj,
                const glm::mat4& view,
                const glm::vec3& viewPosition);

private:
    entt::registry* mRegistry{};

    Buffer               mPerFrameBuffer;
    Buffer               mLightDataBuffer;
    Shader               mVertMeshShader;
    Shader               mFragMeshShader;
    GraphicPipeline      mMeshPipeline;
    VkDescriptorSet      mMeshPipelineDescriptorSet0;
    VulkanDescriptorPool mDescriptorPool;
};
