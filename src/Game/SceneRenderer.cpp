#include "SceneRenderer.h"

#include "Renderer.h"

#include "vulkan/VulkanContext.h"
#include "vulkan/VulkanShaderProgram.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

struct PerFrameData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 viewPosition;
    float _pad;
    glm::vec4 ambientLight;
};
static_assert(sizeof(PerFrameData) == sizeof(float) * 40);

struct LightData {
    glm::vec4 position;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float constant;
    float linear;
    float quadratic;
};
static_assert(sizeof(LightData) == sizeof(float) * 19);

struct PushData {
    glm::mat4 transform;
    glm::mat4 normalMatrix;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec2 texScale;
    float shininess;
};
static_assert(sizeof(PushData) == sizeof(float) * 47);

SceneRenderer::SceneRenderer() {
     mDescriptorPool.init();

        mMeshShader = VulkanShaderProgram::CreateFromSpirv({"./shaders/mesh_vert.spv", "./shaders/mesh_frag.spv"});
        bool a = mMeshShader->hasShaderStage(VK_SHADER_STAGE_VERTEX_BIT);
        bool b = mMeshShader->hasShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT);
        bool c = mMeshShader->hasPushConstant();


    // Mesh pipeline
    {
        mPerFrameBuffer  = VulkanContext::createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(PerFrameData), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        mLightDataBuffer = VulkanContext::createBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(LightData), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        mMeshPipeline =
            VulkanContext::createGraphicPipeline(mMeshShader, true, true, true);

        //VulkanContext::setDebugObjectName((uint64_t)mMeshPipeline.descriptorSetLayout[0], VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
        //                                  "MeshPipelineDescriptorSet0Layout");
        //VulkanContext::setDebugObjectName((uint64_t)mVertMeshShader.shaderModule, VK_OBJECT_TYPE_SHADER_MODULE,
        //                                  "VertMeshShader");
        //VulkanContext::setDebugObjectName((uint64_t)mFragMeshShader.shaderModule, VK_OBJECT_TYPE_SHADER_MODULE,
        //                                  "FragMeshShader");
        VulkanContext::setDebugObjectName((uint64_t)mMeshPipeline.pipeline, VK_OBJECT_TYPE_PIPELINE,
                                          "meshPipeline");
        VulkanContext::setDebugObjectName((uint64_t)mMeshPipeline.pipelineLayout,
                                          VK_OBJECT_TYPE_PIPELINE_LAYOUT, "meshpipelineLayout");
    }
}

SceneRenderer::~SceneRenderer() {
    mDescriptorPool.destroy();

    vmaDestroyBuffer(VulkanContext::getVmaAllocator(), mPerFrameBuffer.buffer, mPerFrameBuffer.allocation);
    vmaDestroyBuffer(VulkanContext::getVmaAllocator(), mLightDataBuffer.buffer, mLightDataBuffer.allocation);
    mMeshPipeline.destroy();
    mMeshShader.reset();
}

void SceneRenderer::render(entt::registry*  registry,
                           VkCommandBuffer  cmd,
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
        auto view           = mRegistry->view<CTransform, CPointLight>();
        for (auto [entity, ctrans, pointLight] : view.each()) {
            LightData lightData{};
            lightData.position = glm::vec4(ctrans.position, 1.0f);
            lightData.ambient   = glm::vec4(pointLight.ambient, 1.0f);
            lightData.diffuse   = glm::vec4(pointLight.diffuse, 1.0f);
            lightData.specular  = glm::vec4(pointLight.specular, 1.0f);
            lightData.constant  = pointLight.constant;
            lightData.linear    = pointLight.linear;
            lightData.quadratic = pointLight.quadratic;
            void* pData{};
            vmaMapMemory(VulkanContext::getVmaAllocator(), mLightDataBuffer.allocation, &pData);
            std::memcpy(pData, &lightData, sizeof(lightData));
            vmaUnmapMemory(VulkanContext::getVmaAllocator(), mLightDataBuffer.allocation);
        }
    }

    // render scene
    {
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipeline.pipeline);

        PushData pushData{};
        auto view           = mRegistry->view<CTransform, CMesh, CMaterial>();
        for (auto [entity, ctrans, cmesh, cmat] : view.each()) {

            if(cmat.descriptorSet1 == VK_NULL_HANDLE) {
                cmat.descriptorSet0 = mDescriptorPool.allocate(mMeshPipeline.descriptorSetLayout[0]);
                cmat.descriptorSet1 = mDescriptorPool.allocate(mMeshPipeline.descriptorSetLayout[1]);

                VkDescriptorBufferInfo bufferInfo[2];
                bufferInfo[0].buffer = mPerFrameBuffer.buffer;
                bufferInfo[0].offset = 0;
                bufferInfo[0].range  = VK_WHOLE_SIZE;
                bufferInfo[1].buffer = mLightDataBuffer.buffer;
                bufferInfo[1].offset = 0;
                bufferInfo[1].range  = VK_WHOLE_SIZE;

                VkWriteDescriptorSet writeDescriptorSet[3]{};
                writeDescriptorSet[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet[0].dstSet          = cmat.descriptorSet0;
                writeDescriptorSet[0].dstBinding      = 0;
                writeDescriptorSet[0].descriptorCount = 1;
                writeDescriptorSet[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDescriptorSet[0].pBufferInfo     = bufferInfo;
                writeDescriptorSet[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet[1].dstSet          = cmat.descriptorSet0;
                writeDescriptorSet[1].dstBinding      = 1;
                writeDescriptorSet[1].descriptorCount = 1;
                writeDescriptorSet[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDescriptorSet[1].pBufferInfo     = &bufferInfo[1];
                vkUpdateDescriptorSets(VulkanContext::getDevice(), 2, writeDescriptorSet, 0, nullptr);

                VkDescriptorImageInfo descriptorImageInfo;
                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                descriptorImageInfo.imageView   = cmat.diffuseMap.view;
                descriptorImageInfo.sampler     = cmat.diffuseMap.sampler;

                VkWriteDescriptorSet writeDescriptorSet2[1]{};
                writeDescriptorSet2[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet2[0].dstSet          = cmat.descriptorSet1;
                writeDescriptorSet2[0].dstBinding      = 2;
                writeDescriptorSet2[0].descriptorCount = 1;
                writeDescriptorSet2[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writeDescriptorSet2[0].pImageInfo      = &descriptorImageInfo;
                vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, writeDescriptorSet2, 0, nullptr);

                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                descriptorImageInfo.imageView   = cmat.specularMap.view;
                descriptorImageInfo.sampler     = cmat.specularMap.sampler;
                writeDescriptorSet2[0].dstBinding =3;
                vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, writeDescriptorSet2, 0, nullptr);

                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                descriptorImageInfo.imageView   = cmat.normalMap.view;
                descriptorImageInfo.sampler     = cmat.normalMap.sampler;
                writeDescriptorSet2[0].dstBinding =4;
                vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, writeDescriptorSet2, 0, nullptr);
            }

            auto translateMat  = glm::translate(glm::mat4(1), ctrans.position);
            auto rotationMat   = glm::eulerAngleYXZ(glm::radians(ctrans.rotation.y), glm::radians(ctrans.rotation.x), glm::radians(ctrans.rotation.z));
            auto scaleMat      = glm::scale(glm::mat4(1), ctrans.scale);
            pushData.transform     = translateMat * rotationMat * scaleMat;
            pushData.normalMatrix  = glm::transpose(glm::inverse(pushData.transform));
            pushData.ambient   = cmat.ambient;
            pushData.diffuse   = cmat.diffuse;
            pushData.specular  = cmat.specular;
            pushData.shininess = cmat.shininess;
            pushData.texScale  = cmat.texScale;

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    mMeshPipeline.pipelineLayout, 0 /*firstSet*/, 1 /*nbSet*/,
                                    &cmat.descriptorSet0, 0, nullptr);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    mMeshPipeline.pipelineLayout, 1 /*firstSet*/, 1 /*nbSet*/,
                                    &cmat.descriptorSet1, 0, nullptr);
            vkCmdPushConstants(cmd, mMeshPipeline.pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(pushData), reinterpret_cast<void*>(&pushData));

            Renderer::DrawMesh(cmd, cmesh.mesh);
        }
    }
}
