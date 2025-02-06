#include "SceneRenderer.h"

#include "Renderer.h"

#include "vulkan/VulkanContext.h"

#include <glm/gtc/matrix_transform.hpp>
#include <spirv_mesh_frag_glsl.h>
#include <spirv_mesh_vert_glsl.h>

struct PushData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
    float     color[4];
};

SceneRenderer::SceneRenderer() {
     mDescriptorPool.init();

    // Mesh pipeline
    {
        mVertMeshShader = VulkanContext::createShaderModule(spirv_mesh_vert_glsl);
        mFragMeshShader = VulkanContext::createShaderModule(spirv_mesh_frag_glsl);
        mMeshPipeline =
            VulkanContext::createGraphicPipeline(mVertMeshShader, mFragMeshShader, true, true);
        mMeshPipelineDescriptorSet0 =
            mDescriptorPool.allocate(mMeshPipeline.descriptorSetLayout[0]);

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
    mMeshPipeline.destroy();
}

void SceneRenderer::render(entt::registry*  registry,
                           VkCommandBuffer  cmd,
                           Texture          texture,
                           const glm::mat4& proj,
                           const glm::mat4& view) {
    mRegistry = registry;

    {
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipeline.pipeline);

        VkDescriptorImageInfo descriptorImageInfo;
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        descriptorImageInfo.imageView   = texture.view;
        descriptorImageInfo.sampler     = texture.sampler;

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet          = mMeshPipelineDescriptorSet0;
        writeDescriptorSet.dstBinding      = 0;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.pImageInfo      = &descriptorImageInfo;

        vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);

        PushData pushData;
        pushData.projection = proj;
        pushData.view       = view;
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
