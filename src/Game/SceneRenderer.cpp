#include "SceneRenderer.h"

#include "Renderer.h"

#include "vulkan/VulkanContext.h"

#include <glm/gtc/matrix_transform.hpp>
#include <spirv_mesh_frag_glsl.h>
#include <spirv_mesh_vert_glsl.h>

struct PerFrameData {
    glm::mat4 projection;
    glm::mat4 view;
};

struct PushData {
    glm::mat4 model;
    float     color[4];
};

SceneRenderer::SceneRenderer() {
     mDescriptorPool.init();

    // Mesh pipeline
    {
        mPerFrameBuffer = VulkanContext::createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(PerFrameData), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        mVertMeshShader = VulkanContext::createShaderModule(spirv_mesh_vert_glsl);
        mFragMeshShader = VulkanContext::createShaderModule(spirv_mesh_frag_glsl);
        mMeshPipeline =
            VulkanContext::createGraphicPipeline(mVertMeshShader, mFragMeshShader, true, true);
        mMeshPipelineDescriptorSet0 =
            mDescriptorPool.allocate(mMeshPipeline.descriptorSetLayout[0]);

        VulkanContext::setDebugObjectName((uint64_t)mMeshPipeline.descriptorSetLayout[0], VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                                          "MeshPipelineDescriptorSet0Layout");
        VulkanContext::setDebugObjectName((uint64_t)mVertMeshShader.shaderModule, VK_OBJECT_TYPE_DESCRIPTOR_SET,
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
    mMeshPipeline.destroy();
}

void SceneRenderer::render(entt::registry*  registry,
                           VkCommandBuffer  cmd,
                           Texture          texture,
                           const glm::mat4& proj,
                           const glm::mat4& view) {
    mRegistry = registry;

    // upload per frame data
    {
        PerFrameData perFrameData{};
        perFrameData.projection = proj;
        perFrameData.view = view;

        void* pData{};
        vmaMapMemory(VulkanContext::getVmaAllocator(), mPerFrameBuffer.allocation, &pData);
        std::memcpy(pData, &perFrameData, sizeof(perFrameData));
        vmaUnmapMemory(VulkanContext::getVmaAllocator(), mPerFrameBuffer.allocation);
    }

    // render scene
    {
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipeline.pipeline);

        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = mPerFrameBuffer.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range  = VK_WHOLE_SIZE;

        VkDescriptorImageInfo descriptorImageInfo;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        descriptorImageInfo.imageView   = texture.view;
        descriptorImageInfo.sampler     = texture.sampler;

        VkWriteDescriptorSet writeDescriptorSet[2]{};
        writeDescriptorSet[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[0].dstSet          = mMeshPipelineDescriptorSet0;
        writeDescriptorSet[0].dstBinding      = 0;
        writeDescriptorSet[0].descriptorCount = 1;
        writeDescriptorSet[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[0].pBufferInfo     = &bufferInfo;
        writeDescriptorSet[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[1].dstSet          = mMeshPipelineDescriptorSet0;
        writeDescriptorSet[1].dstBinding      = 1;
        writeDescriptorSet[1].descriptorCount = 1;
        writeDescriptorSet[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet[1].pImageInfo      = &descriptorImageInfo;
        vkUpdateDescriptorSets(VulkanContext::getDevice(), 2, writeDescriptorSet, 0, nullptr);

        PushData pushData{};
        auto view           = mRegistry->view<CTransform, CMesh, CMaterial>();
        for (auto [entity, ctrans, cmesh, cmat] : view.each()) {
            pushData.model    = glm::translate(glm::mat4(1), ctrans.position);
            pushData.color[0] = cmat.color.x;
            pushData.color[1] = cmat.color.y;
            pushData.color[2] = cmat.color.z;
            pushData.color[3] = cmat.color.w;

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
