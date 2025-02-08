#pragma once
#include "vulkan/vulkan.h"
#include "vulkan/VulkanDescriptorPool.h"
#include "Mesh.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

struct CTransform {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
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

class SceneRenderer {
public:
    SceneRenderer();
    ~SceneRenderer();

    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;

    SceneRenderer(SceneRenderer&&) = delete;
    SceneRenderer& operator=(SceneRenderer&&) = delete;

    void render(entt::registry*, VkCommandBuffer cmd, Texture texture, const glm::mat4& proj, const glm::mat4& view,
                           const glm::vec3& viewPosition);

private:
    entt::registry* mRegistry{};

    Buffer           mPerFrameBuffer;
    Buffer           mLightDataBuffer;
    Shader           mVertMeshShader;
    Shader           mFragMeshShader;
    GraphicPipeline  mMeshPipeline;
    VkDescriptorSet  mMeshPipelineDescriptorSet0;
    VulkanDescriptorPool mDescriptorPool;
};
