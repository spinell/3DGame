#include "SceneRenderer.h"

#include "Renderer.h"

#include "vulkan/VulkanContext.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <spirv_mesh_frag_glsl.h>
#include <spirv_mesh_vert_glsl.h>

struct PerFrameData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 viewPosition;
    float _pad;
    glm::vec4 ambientLight;
};
static_assert(sizeof(PerFrameData) == sizeof(float) * 40);

struct LightData {
    glm::vec4 direction;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};
static_assert(sizeof(LightData) == sizeof(float) * 16);

struct PushData {
    glm::mat4 model;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float shininess;
};
static_assert(sizeof(PushData) == sizeof(float) * 29);

SceneRenderer::SceneRenderer() {
     mDescriptorPool.init();

    // Mesh pipeline
    {
        mPerFrameBuffer  = VulkanContext::createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(PerFrameData), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        mLightDataBuffer = VulkanContext::createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(LightData), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        mVertMeshShader = VulkanContext::createShaderModule(spirv_mesh_vert_glsl);
        mFragMeshShader = VulkanContext::createShaderModule(spirv_mesh_frag_glsl);
        mMeshPipeline =
            VulkanContext::createGraphicPipeline(mVertMeshShader, mFragMeshShader, true, true, true);
        mMeshPipelineDescriptorSet0 =
            mDescriptorPool.allocate(mMeshPipeline.descriptorSetLayout[0]);

        VulkanContext::setDebugObjectName((uint64_t)mMeshPipeline.descriptorSetLayout[0], VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                                          "MeshPipelineDescriptorSet0Layout");
        VulkanContext::setDebugObjectName((uint64_t)mMeshPipelineDescriptorSet0, VK_OBJECT_TYPE_DESCRIPTOR_SET,
                                          "MeshPipelineDescriptorSet0");
        VulkanContext::setDebugObjectName((uint64_t)mVertMeshShader.shaderModule, VK_OBJECT_TYPE_SHADER_MODULE,
                                          "VertMeshShader");
        VulkanContext::setDebugObjectName((uint64_t)mFragMeshShader.shaderModule, VK_OBJECT_TYPE_SHADER_MODULE,
                                          "FragMeshShader");
        VulkanContext::setDebugObjectName((uint64_t)mMeshPipeline.pipeline, VK_OBJECT_TYPE_PIPELINE,
                                          "meshPipeline");
        VulkanContext::setDebugObjectName((uint64_t)mMeshPipeline.pipelineLayout,
                                          VK_OBJECT_TYPE_PIPELINE_LAYOUT, "meshpipelineLayout");
        VulkanContext::setDebugObjectName((uint64_t)mMeshPipelineDescriptorSet0,
                                          VK_OBJECT_TYPE_DESCRIPTOR_SET,
                                          "meshPipelineDescriptorSet0");
    }
}

SceneRenderer::~SceneRenderer() {
    mDescriptorPool.destroy();

    vkDestroyShaderModule(VulkanContext::getDevice(), mVertMeshShader.shaderModule, nullptr);
    vkDestroyShaderModule(VulkanContext::getDevice(), mFragMeshShader.shaderModule, nullptr);
    vmaDestroyBuffer(VulkanContext::getVmaAllocator(), mPerFrameBuffer.buffer, mPerFrameBuffer.allocation);
    vmaDestroyBuffer(VulkanContext::getVmaAllocator(), mLightDataBuffer.buffer, mLightDataBuffer.allocation);
    mMeshPipeline.destroy();
}

void SceneRenderer::render(entt::registry*  registry,
                           VkCommandBuffer  cmd,
                           Texture          texture,
                           const glm::mat4& proj,
                           const glm::mat4& view,
                           const glm::vec3& viewPosition) {
    mRegistry = registry;

    // upload per frame data
    {
        PerFrameData perFrameData{};
        perFrameData.projection = proj;
        perFrameData.view = view;
        perFrameData.viewPosition = viewPosition;
        perFrameData.ambientLight = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);

        void* pData{};
        vmaMapMemory(VulkanContext::getVmaAllocator(), mPerFrameBuffer.allocation, &pData);
        std::memcpy(pData, &perFrameData, sizeof(perFrameData));
        vmaUnmapMemory(VulkanContext::getVmaAllocator(), mPerFrameBuffer.allocation);
    }

    {
        LightData lightData{};
        lightData.ambient   = {0.2f, 0.2f, 0.2f, 1.0f};
        lightData.diffuse   = {0.5f, 0.5f, 0.5f, 1.0f};
        lightData.specular  = {0.5f, 0.5f, 0.5f, 1.0f};
        lightData.direction = glm::normalize(glm::vec4(0.0f, -1.0f, 1.0f, 0.0f));
        void* pData{};
        vmaMapMemory(VulkanContext::getVmaAllocator(), mLightDataBuffer.allocation, &pData);
        std::memcpy(pData, &lightData, sizeof(lightData));
        vmaUnmapMemory(VulkanContext::getVmaAllocator(), mLightDataBuffer.allocation);
    }

    // render scene
    {
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipeline.pipeline);

        VkDescriptorBufferInfo bufferInfo[2];
        bufferInfo[0].buffer = mPerFrameBuffer.buffer;
        bufferInfo[0].offset = 0;
        bufferInfo[0].range  = VK_WHOLE_SIZE;
        bufferInfo[1].buffer = mLightDataBuffer.buffer;
        bufferInfo[1].offset = 0;
        bufferInfo[1].range  = VK_WHOLE_SIZE;

        VkDescriptorImageInfo descriptorImageInfo;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        descriptorImageInfo.imageView   = texture.view;
        descriptorImageInfo.sampler     = texture.sampler;

        VkWriteDescriptorSet writeDescriptorSet[3]{};
        writeDescriptorSet[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[0].dstSet          = mMeshPipelineDescriptorSet0;
        writeDescriptorSet[0].dstBinding      = 0;
        writeDescriptorSet[0].descriptorCount = 1;
        writeDescriptorSet[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[0].pBufferInfo     = bufferInfo;
        writeDescriptorSet[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[1].dstSet          = mMeshPipelineDescriptorSet0;
        writeDescriptorSet[1].dstBinding      = 1;
        writeDescriptorSet[1].descriptorCount = 1;
        writeDescriptorSet[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[1].pBufferInfo     = &bufferInfo[1];
        writeDescriptorSet[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[2].dstSet          = mMeshPipelineDescriptorSet0;
        writeDescriptorSet[2].dstBinding      = 2;
        writeDescriptorSet[2].descriptorCount = 1;
        writeDescriptorSet[2].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet[2].pImageInfo      = &descriptorImageInfo;
        vkUpdateDescriptorSets(VulkanContext::getDevice(), 3, writeDescriptorSet, 0, nullptr);

        PushData pushData{};
        auto view           = mRegistry->view<CTransform, CMesh, CMaterial>();
        for (auto [entity, ctrans, cmesh, cmat] : view.each()) {
            auto translateMat  = glm::translate(glm::mat4(1), ctrans.position);
            auto rotationMat   = glm::eulerAngleYXZ(glm::radians(ctrans.rotation.y), glm::radians(ctrans.rotation.x), glm::radians(ctrans.rotation.z));
            auto scaleMat      = glm::scale(glm::mat4(1), ctrans.scale);
            pushData.model     = translateMat * rotationMat * scaleMat;
            pushData.ambient   = cmat.ambient;
            pushData.diffuse   = cmat.diffuse;
            pushData.specular  = cmat.specular;
            pushData.shininess = cmat.shininess;

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    mMeshPipeline.pipelineLayout, 0 /*firstSet*/, 1 /*nbSet*/,
                                    &mMeshPipelineDescriptorSet0, 0, nullptr);

            vkCmdPushConstants(cmd, mMeshPipeline.pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(pushData), reinterpret_cast<void*>(&pushData));

            Renderer::DrawMesh(cmd, cmesh.mesh);
        }
    }
}
