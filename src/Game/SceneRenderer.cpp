#include "SceneRenderer.h"

#include "Renderer.h"

#include "vulkan/VulkanContext.h"
#include "vulkan/VulkanTexture.h"
#include "vulkan/VulkanShaderProgram.h"
#include "vulkan/VulkanGraphicPipeline.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

struct PerFrameData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 viewProjection;
    glm::vec3 viewPosition;
    float _pad;
    glm::vec4 ambientLight;
    int useBlinnPhong;
    int useGammaCorrection = true;
    float gamma = 2.2f;
};
static_assert(offsetof(PerFrameData, projection) == 0);
static_assert(offsetof(PerFrameData, view) == 64);
static_assert(offsetof(PerFrameData, viewProjection) == 128);
static_assert(offsetof(PerFrameData, viewPosition) == 192);
static_assert(offsetof(PerFrameData, ambientLight) == 208);
static_assert(offsetof(PerFrameData, useBlinnPhong) == 224);
static_assert(offsetof(PerFrameData, useGammaCorrection) == 228);
static_assert(offsetof(PerFrameData, gamma) == 232);

struct PointLight {
    glm::vec4 position;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float range;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    float pad0;
    float pad1;
    float pad2;
};
//static_assert(sizeof(PointLight) == sizeof(float) * 22);
struct DirectionalLight {
    glm::vec4 color;
    glm::vec4 direction;
};
struct SpotLight {
    glm::vec4 color;
    glm::vec4 position;
    glm::vec4 direction;
    float     range;
    float     cutOffInner;
    float     cutOffOuter;
    float     pad1;
};
struct LightData {
    uint32_t nbLight;
    uint32_t nbDirectionalLight;
    uint32_t nbSpotLight;
    uint32_t _pad2;
    PointLight lights[512];
    DirectionalLight directionalLight[4];
    SpotLight        spotLights[4];
};


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
    VulkanContext::setDebugObjectName((uint64_t)mMeshShader->getPipelineLayout(), VK_OBJECT_TYPE_PIPELINE_LAYOUT, "MeshPipelineLayout" );
    bool a = mMeshShader->hasShaderStage(VK_SHADER_STAGE_VERTEX_BIT);
    bool b = mMeshShader->hasShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT);
    bool c = mMeshShader->hasPushConstant();


    // Mesh pipeline
    {
        VulkanBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.name           = "PerFrameData";
        bufferCreateInfo.sizeInByte     = sizeof(PerFrameData);
        bufferCreateInfo.usage          = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferCreateInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mPerFrameBuffer                 = VulkanBuffer::Create(bufferCreateInfo);

        bufferCreateInfo.name           = "LightData";
        bufferCreateInfo.sizeInByte     = sizeof(LightData);
        bufferCreateInfo.usage          = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferCreateInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mLightDataBuffer                = VulkanBuffer::Create(bufferCreateInfo);

        VulkanGraphicPipelineCreateInfo createInfo{};
        createInfo.name = "meshPipeline";
        createInfo.shader = mMeshShader;
        createInfo.cullMode = VK_CULL_MODE_NONE;
        createInfo.vertexStride = 44;
        createInfo.vertexInput = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 * 3}, // position
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, 4 * 3}, // normal
            {2, 0, VK_FORMAT_R32G32B32_SFLOAT, 4 * 6}, // tangent
            {3, 0, VK_FORMAT_R32G32_SFLOAT, 4 * 9}     // tex
        };
        mMeshPipeline    = VulkanGraphicPipeline::Create(createInfo);
        mDescriptorSet   = mDescriptorPool.allocate(mMeshPipeline->getDescriptorSetLayouts()[0]);

        VkDescriptorBufferInfo bufferInfo[2];
        bufferInfo[0].buffer = mPerFrameBuffer->getBuffer();
        bufferInfo[0].offset = 0;
        bufferInfo[0].range  = VK_WHOLE_SIZE;
        bufferInfo[1].buffer = mLightDataBuffer->getBuffer();
        bufferInfo[1].offset = 0;
        bufferInfo[1].range  = VK_WHOLE_SIZE;

        VkWriteDescriptorSet writeDescriptorSet[3]{};
        writeDescriptorSet[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[0].dstSet          = mDescriptorSet;
        writeDescriptorSet[0].dstBinding      = 0;
        writeDescriptorSet[0].descriptorCount = 1;
        writeDescriptorSet[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[0].pBufferInfo     = bufferInfo;
        writeDescriptorSet[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[1].dstSet          = mDescriptorSet;
        writeDescriptorSet[1].dstBinding      = 1;
        writeDescriptorSet[1].descriptorCount = 1;
        writeDescriptorSet[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[1].pBufferInfo     = &bufferInfo[1];
        vkUpdateDescriptorSets(VulkanContext::getDevice(), 2, writeDescriptorSet, 0, nullptr);

        //VulkanContext::setDebugObjectName((uint64_t)mMeshPipeline.descriptorSetLayout[0], VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
        //                                  "MeshPipelineDescriptorSet0Layout");
        //VulkanContext::setDebugObjectName((uint64_t)mVertMeshShader.shaderModule, VK_OBJECT_TYPE_SHADER_MODULE,
        //                                  "VertMeshShader");
        //VulkanContext::setDebugObjectName((uint64_t)mFragMeshShader.shaderModule, VK_OBJECT_TYPE_SHADER_MODULE,
        //                                  "FragMeshShader");
        VulkanContext::setDebugObjectName((uint64_t)mMeshPipeline->getPipelineLayout(),
                                          VK_OBJECT_TYPE_PIPELINE_LAYOUT, "meshpipelineLayout");
    }

    // Skybox
    {
        mSkyboxShader   = VulkanShaderProgram::CreateFromSpirv({"./shaders/skybox_vert.spv", "./shaders/skybox_frag.spv"});
        VulkanContext::setDebugObjectName((uint64_t)mSkyboxShader->getPipelineLayout(), VK_OBJECT_TYPE_PIPELINE_LAYOUT, "SkyboxPipelineLayout" );
        VulkanGraphicPipelineCreateInfo createInfo{};
        createInfo.name = "skybox";
        createInfo.shader = mSkyboxShader;
        createInfo.cullMode=VK_CULL_MODE_BACK_BIT;
        createInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        createInfo.vertexStride = 12;
        createInfo.vertexInput = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 * 3}, // position
        };
        mSkyboxPipeline = VulkanGraphicPipeline::Create(createInfo);

        // Define the cube vertices
        const float cubeVertices[] = {
            -1.0f, -1.0f, -1.0f, // left  - bottom - far	 index 0
            +0.0f, -1.0f, -1.0f, // right - bottom - far     index 1
            +0.0f, +1.0f, -1.0f, // right - top    - far     index 2
            -1.0f, +1.0f, -1.0f, // left  - top    - far     index 3
            -1.0f, -1.0f, +1.0f, // left  - bottom - near    index 4
            +1.0f, -1.0f, +1.0f, // right - bottom - near    index 5
            +1.0f, +1.0f, +1.0f, // right - top    - near    index 6
            -1.0f, +1.0f, +1.0f  // left  - top    - near    index 7
        };

        // define the cube indices in ccw order
        const unsigned cubeIndices[] = {
            0, 1, 2, 2, 3, 0, // far plane
            2, 1, 5, 5, 6, 2, // right plane
            3, 7, 4, 4, 0, 3, // left plane
            6, 5, 4, 4, 7, 6, // near plane
            7, 3, 2, 2, 6, 7, // top plane
            0, 4, 5, 5, 1, 0  // bottom plane
        };

        VulkanBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.name           = "SkyBoxVB";
        bufferCreateInfo.sizeInByte     = sizeof(cubeVertices);
        bufferCreateInfo.usage          = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferCreateInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mSkyBoxVertexBuffer = VulkanBuffer::Create(bufferCreateInfo);

        bufferCreateInfo.name           = "SkyBoxIB";
        bufferCreateInfo.sizeInByte     = sizeof(cubeIndices);
        bufferCreateInfo.usage          = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufferCreateInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mSkyBoxIndexBuffer = VulkanBuffer::Create(bufferCreateInfo);

        mSkyBoxVertexBuffer->writeData(cubeVertices, sizeof(cubeVertices));
        mSkyBoxIndexBuffer->writeData(cubeIndices, sizeof(cubeIndices));
    }

    // Draw mesh AABB
    {
        mDrawMeshAABB.shader = VulkanShaderProgram::CreateFromSpirv({"./shaders/mesh_aabb_vert.spv", "./shaders/mesh_aabb_geo.spv", "./shaders/mesh_aabb_frag.spv"});
        VulkanContext::setDebugObjectName((uint64_t)mDrawMeshAABB.shader->getPipelineLayout(), VK_OBJECT_TYPE_PIPELINE_LAYOUT, "MeshAABBPipelineLayout" );
        assert(mDrawMeshAABB.shader);

        VulkanGraphicPipelineCreateInfo createInfo{};
        createInfo.name = "MeshAABB";
        createInfo.shader = mDrawMeshAABB.shader;
        createInfo.primitiveTopology =VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        createInfo.cullMode=VK_CULL_MODE_NONE;
        mDrawMeshAABB.pipeline = VulkanGraphicPipeline::Create(createInfo);
        assert(mDrawMeshAABB.pipeline);

        mDrawMeshAABB.descriptorSet = mDescriptorPool.allocate(mDrawMeshAABB.pipeline->getDescriptorSetLayouts()[0]);
        VulkanContext::setDebugObjectName((uint64_t)mDrawMeshAABB.descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, "MeshAABB" );

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mPerFrameBuffer->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range  = VK_WHOLE_SIZE;

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet          = mDrawMeshAABB.descriptorSet;
        writeDescriptorSet.dstBinding      = 0;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.pBufferInfo     = &bufferInfo;
        vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);
    }
}

SceneRenderer::~SceneRenderer() {
    mDescriptorPool.destroy();

    mPerFrameBuffer.reset();
    mLightDataBuffer.reset();
    mSkyBoxVertexBuffer.reset();
    mSkyBoxIndexBuffer.reset();
    mMeshPipeline.reset();
    mSkyboxPipeline.reset();
    mMeshShader.reset();
    mSkyboxShader.reset();
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
        perFrameData.viewProjection = proj * view;
        perFrameData.viewPosition = viewPosition;
        perFrameData.ambientLight = glm::vec4(mAmbientLight, 1.0f);
        perFrameData.useBlinnPhong = mUseBlinnPhong;
        perFrameData.useGammaCorrection = mUseGammaCorrection;
        perFrameData.gamma = mGamma;
        mPerFrameBuffer->writeData(&perFrameData, sizeof(perFrameData));
    }

    //
    // Point Light
    //
    {
        auto view = mRegistry->view<CTransform, CPointLight>();
        LightData lightData{};
        lightData.nbLight = 0;
        for (auto [entity, ctrans, pointLight] : view.each()) {
            if(!pointLight.enable) {
                continue;
            }
            PointLight& light = lightData.lights[lightData.nbLight];
            light.position = glm::vec4(ctrans.position, 1.0f);
            light.ambient   = glm::vec4(pointLight.ambient, 1.0f);
            light.diffuse   = glm::vec4(pointLight.diffuse, 1.0f);
            light.specular  = glm::vec4(pointLight.specular, 1.0f);
            light.constant  = pointLight.constant;
            light.linear    = pointLight.linear;
            light.quadratic = pointLight.quadratic;
            light.range     = pointLight.range;
            light.intensity = pointLight.intensity;
            lightData.nbLight++;
        }

        lightData.nbDirectionalLight = 0;
        for (auto [entity, directionalLight] : mRegistry->view<CDirectionalLight>().each()) {
            if(!directionalLight.enable) {
                continue;
            }
            DirectionalLight& light = lightData.directionalLight[lightData.nbDirectionalLight];
            light.color     = glm::vec4(directionalLight.color, 1.0f);
            light.direction = glm::vec4(directionalLight.direction, 1.0f);
            lightData.nbDirectionalLight++;
        }

        lightData.nbSpotLight = 0;
        for (auto [entity, transform, spotLight] : mRegistry->view<CTransform, CSpotLight>().each()) {
            if(!spotLight.enable) {
                continue;
            }
            SpotLight& light = lightData.nbSpotLight[lightData.spotLights];
            light.position  = glm::vec4(transform.position, 1.0f);
            light.color     = glm::vec4(spotLight.color, 1.0f);
            light.direction = glm::vec4(spotLight.direction, 1.0f);
            light.range     = spotLight.range;
            light.cutOffInner = glm::cos(glm::radians(spotLight.cutOffAngle));
            light.cutOffOuter = glm::cos(glm::radians(spotLight.cutOffAngle+12.5f));
            lightData.nbSpotLight++;
        }

        mLightDataBuffer->writeData(&lightData,sizeof(lightData));
    }



    // render scene
    {
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mMeshPipeline->getPipeline());

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
            mMeshPipeline->getPipelineLayout(), 0 /*firstSet*/, 1 /*nbSet*/,
            &mDescriptorSet, 0, nullptr);

        PushData pushData{};
        auto view           = mRegistry->view<CTransform, CMesh, CMaterial>();
        for (auto [entity, ctrans, cmesh, cmat] : view.each()) {

            if(cmat.descriptorSet1 == VK_NULL_HANDLE) {
                cmat.descriptorSet1 = mDescriptorPool.allocate(mMeshPipeline->getDescriptorSetLayouts()[1]);

                VkDescriptorImageInfo descriptorImageInfo;
                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                descriptorImageInfo.imageView   = cmat.diffuseMap->getImageView();
                descriptorImageInfo.sampler     = cmat.diffuseMap->getSampler();

                VkWriteDescriptorSet writeDescriptorSet2[1]{};
                writeDescriptorSet2[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet2[0].dstSet          = cmat.descriptorSet1;
                writeDescriptorSet2[0].dstBinding      = 2;
                writeDescriptorSet2[0].descriptorCount = 1;
                writeDescriptorSet2[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writeDescriptorSet2[0].pImageInfo      = &descriptorImageInfo;
                vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, writeDescriptorSet2, 0, nullptr);

                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                descriptorImageInfo.imageView   = cmat.specularMap->getImageView();
                descriptorImageInfo.sampler     = cmat.specularMap->getSampler();
                writeDescriptorSet2[0].dstBinding =3;
                vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, writeDescriptorSet2, 0, nullptr);

                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                descriptorImageInfo.imageView   = cmat.normalMap->getImageView();
                descriptorImageInfo.sampler     = cmat.normalMap->getSampler();
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
                                    mMeshPipeline->getPipelineLayout(), 1 /*firstSet*/, 1 /*nbSet*/,
                                    &cmat.descriptorSet1, 0, nullptr);
            vkCmdPushConstants(cmd, mMeshPipeline->getPipelineLayout(),
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(pushData), reinterpret_cast<void*>(&pushData));

            Renderer::DrawMesh(cmd, cmesh.mesh);
        }
    }

    // skybox
    {
        if(auto* skybox = mRegistry->ctx().find<CSkyBox>()) {
            if(!mSkyBoxDescriptorSet1) {
                mSkyBoxDescriptorSet0= mDescriptorPool.allocate(mSkyboxShader->getDescriptorSetLayouts()[0]);
                mSkyBoxDescriptorSet1= mDescriptorPool.allocate(mSkyboxShader->getDescriptorSetLayouts()[1]);

                VkDescriptorBufferInfo bufferInfo[2];
                bufferInfo[0].buffer = mPerFrameBuffer->getBuffer();
                bufferInfo[0].offset = 0;
                bufferInfo[0].range  = VK_WHOLE_SIZE;

                VkWriteDescriptorSet writeDescriptorSet[1]{};
                writeDescriptorSet[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet[0].dstSet          = mSkyBoxDescriptorSet0;
                writeDescriptorSet[0].dstBinding      = 0;
                writeDescriptorSet[0].descriptorCount = 1;
                writeDescriptorSet[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writeDescriptorSet[0].pBufferInfo     = bufferInfo;
                vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, writeDescriptorSet, 0, nullptr);

                VkDescriptorImageInfo descriptorImageInfo;
                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                descriptorImageInfo.imageView   = skybox->texture->getImageView();
                descriptorImageInfo.sampler     = skybox->texture->getSampler();

                VkWriteDescriptorSet writeDescriptorSet2[1]{};
                writeDescriptorSet2[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet2[0].dstSet          = mSkyBoxDescriptorSet1;
                writeDescriptorSet2[0].dstBinding      = 0;
                writeDescriptorSet2[0].descriptorCount = 1;
                writeDescriptorSet2[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writeDescriptorSet2[0].pImageInfo      = &descriptorImageInfo;
                vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, writeDescriptorSet2, 0, nullptr);
            }
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mSkyboxPipeline->getPipelineLayout(), 0 /*firstSet*/, 1 /*nbSet*/, &mSkyBoxDescriptorSet0, 0, nullptr);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mSkyboxPipeline->getPipelineLayout(), 1 /*firstSet*/, 1 /*nbSet*/, &mSkyBoxDescriptorSet1, 0, nullptr);

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mSkyboxPipeline->getPipeline());
            //Renderer::DrawMesh(cmd, skyBoxMesh);
            VkDeviceSize offset{};
            VkBuffer buffer = mSkyBoxVertexBuffer->getBuffer();
            vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
            vkCmdBindIndexBuffer(cmd, mSkyBoxIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, 36, 1, 0, 0, 1);
        }
    }

    // Render Mesh AABB
    {
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mDrawMeshAABB.pipeline->getPipeline());
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            mDrawMeshAABB.pipeline->getPipelineLayout(),
            0 /*firstSet*/,
            1 /*nbSet*/,
            &mDrawMeshAABB.descriptorSet,
            0,
            nullptr
        );

        struct {
            glm::mat4 transform;
            glm::vec3 min;
            float _pad0;
            glm::vec3 max;
            float _pad1;
            glm::vec3 color;
        }aabb;
        auto view           = mRegistry->view<CTransform, CMesh>();
        for (auto [entity, ctrans, cmesh] : view.each()) {
            auto translateMat  = glm::translate(glm::mat4(1), ctrans.position);
            auto rotationMat   = glm::eulerAngleYXZ(glm::radians(ctrans.rotation.y), glm::radians(ctrans.rotation.x), glm::radians(ctrans.rotation.z));
            auto scaleMat      = glm::scale(glm::mat4(1), ctrans.scale);
            aabb.transform = translateMat * rotationMat * scaleMat;
#if 1
            aabb.color   = {0.f, 1.f, 0.f};
            aabb.min     = cmesh.mesh.aabbMin;
            aabb.max     = cmesh.mesh.aabbMax;
            vkCmdPushConstants(
                cmd,
                mDrawMeshAABB.pipeline->getPipelineLayout(),
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(aabb),
                reinterpret_cast<void*>(&aabb)
            );

            vkCmdDraw(cmd, 1, 1, 0, 0);
#else
            for(const auto& submesh : cmesh.mesh.subMeshs) {
                aabb.color   = {0.f, 0.f, 1.f};
                aabb.min = submesh.aabbMin;
                aabb.max = submesh.aabbMax;
                vkCmdPushConstants(
                    cmd,
                    mDrawMeshAABB.pipeline->getPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(aabb),
                    reinterpret_cast<void*>(&aabb)
                );
                vkCmdDraw(cmd, 1, 1, 0, 0);
            }
#endif
        }
    }
}
