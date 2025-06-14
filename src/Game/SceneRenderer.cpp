#include "SceneRenderer.h"

#include "Renderer.h"

#include "vulkan/VulkanContext.h"
#include "vulkan/VulkanTexture.h"
#include "vulkan/VulkanShaderProgram.h"
#include "vulkan/VulkanGraphicPipeline.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace {
    void planeNormalize(glm::vec4& v) {
        float LengthSq         = v.x * v.x + v.y * v.y + v.z * v.z;
        float ReciprocalLength = 1.0f / sqrt(LengthSq);
        v.x                    = v.x * ReciprocalLength;
        v.y                    = v.y * ReciprocalLength;
        v.z                    = v.z * ReciprocalLength;
        v.w                    = v.w * ReciprocalLength;
    }
    }; // namespace

struct PerFrameData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 viewProjection;
    glm::vec3 viewPosition;
    float _pad;
    glm::vec4 worldFrustumPlanes[6];
    glm::vec4 ambientLight;
    int useBlinnPhong;
    int useGammaCorrection = true;
    float gamma = 2.2f;
};
static_assert(offsetof(PerFrameData, projection) == 0);
static_assert(offsetof(PerFrameData, view) == 64);
static_assert(offsetof(PerFrameData, viewProjection) == 128);
static_assert(offsetof(PerFrameData, viewPosition) == 192);
//static_assert(offsetof(PerFrameData, ambientLight) == 208);
//static_assert(offsetof(PerFrameData, useBlinnPhong) == 224);
//static_assert(offsetof(PerFrameData, useGammaCorrection) == 228);
//static_assert(offsetof(PerFrameData, gamma) == 232);

struct TerrainSetting {
    float tessFactor[4];
    float insideTessFactor[2];
    float minDistance;
    float maxDistance;
    float minTess;
	float maxTess;
};

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

    // Draw mesh normal
    {
        mDrawMeshNormals.shader = VulkanShaderProgram::CreateFromSpirv({"./shaders/mesh_show_normals_vert.spv", "./shaders/mesh_show_normals_geo.spv", "./shaders/mesh_show_normals_frag.spv"});
        VulkanContext::setDebugObjectName((uint64_t)mDrawMeshNormals.shader->getPipelineLayout(), VK_OBJECT_TYPE_PIPELINE_LAYOUT, "MeshNormalPipelineLayout" );
        assert(mDrawMeshNormals.shader);

        VulkanGraphicPipelineCreateInfo createInfo{};
        createInfo.name = "MeshNormal";
        createInfo.shader = mDrawMeshNormals.shader;
        createInfo.primitiveTopology =VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        createInfo.cullMode=VK_CULL_MODE_NONE;
        createInfo.vertexStride = 44;
        createInfo.vertexInput = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 * 3}, // position
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, 4 * 3}, // normal
            {2, 0, VK_FORMAT_R32G32B32_SFLOAT, 4 * 6}, // tangent
        };
        mDrawMeshNormals.pipeline = VulkanGraphicPipeline::Create(createInfo);
        assert(mDrawMeshNormals.pipeline);

        mDrawMeshNormals.descriptorSet = mDescriptorPool.allocate(mDrawMeshNormals.pipeline->getDescriptorSetLayouts()[0]);
        VulkanContext::setDebugObjectName((uint64_t)mDrawMeshNormals.pipeline->getDescriptorSetLayouts()[0], VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "MeshNormalSetLayout0" );
        VulkanContext::setDebugObjectName((uint64_t)mDrawMeshNormals.descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, "MeshNormal" );

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mPerFrameBuffer->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range  = VK_WHOLE_SIZE;

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet          = mDrawMeshNormals.descriptorSet;
        writeDescriptorSet.dstBinding      = 0;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.pBufferInfo     = &bufferInfo;
        vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);
    }

    // Terrain
    {
        VulkanBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.name           = "TerrainSetting";
        bufferCreateInfo.sizeInByte     = sizeof(TerrainSetting);
        bufferCreateInfo.usage          = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferCreateInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mTerrainSettings                = VulkanBuffer::Create(bufferCreateInfo);

        mDrawTerrain.shader = VulkanShaderProgram::CreateFromSpirv(
            {"./shaders/terrain_vert.spv", "./shaders/terrain_hull.spv", "./shaders/terrain_dom.spv", "./shaders/terrain_frag.spv"});
        VulkanContext::setDebugObjectName((uint64_t)mDrawTerrain.shader->getPipelineLayout(), VK_OBJECT_TYPE_PIPELINE_LAYOUT, "TerrainPipelineLayout");
        VulkanContext::setDebugObjectName((uint64_t)mDrawTerrain.shader->getDescriptorSetLayouts()[0], VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "TerrainDescriptorSet0");
        assert(mDrawTerrain.shader);

        VulkanGraphicPipelineCreateInfo createInfo{};
        createInfo.name              = "Terrain";
        createInfo.shader            = mDrawTerrain.shader;
        createInfo.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        createInfo.cullMode          = VK_CULL_MODE_BACK_BIT;
        createInfo.vertexStride      = sizeof(Terrain::Vertex);
        createInfo.vertexInput       = {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Terrain::Vertex, pos)},  // position
            {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Terrain::Vertex, tex)},     // uv
            {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Terrain::Vertex, boundsY)}, // bound
        };
        mDrawTerrain.pipeline = VulkanGraphicPipeline::Create(createInfo);
        assert(mDrawTerrain.pipeline);

        mDrawTerrain.descriptorSet0 = mDescriptorPool.allocate(mDrawTerrain.pipeline->getDescriptorSetLayouts()[0]);
        VulkanContext::setDebugObjectName((uint64_t)mDrawTerrain.descriptorSet0, VK_OBJECT_TYPE_DESCRIPTOR_SET, "TerrainDescriptorSet0" );

        VkDescriptorBufferInfo bufferInfo[2]{};
        bufferInfo[0].buffer = mPerFrameBuffer->getBuffer();
        bufferInfo[0].offset = 0;
        bufferInfo[0].range  = VK_WHOLE_SIZE;
        bufferInfo[1].buffer = mLightDataBuffer->getBuffer();
        bufferInfo[1].offset = 0;
        bufferInfo[1].range  = VK_WHOLE_SIZE;
        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet          = mDrawTerrain.descriptorSet0;
        writeDescriptorSet.dstBinding      = 0;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.pBufferInfo     = &bufferInfo[0];
        vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);
        writeDescriptorSet.dstBinding      = 1;
        writeDescriptorSet.pBufferInfo     = &bufferInfo[1];
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
        // left
        perFrameData.worldFrustumPlanes[0].x = perFrameData.viewProjection[0][3] + perFrameData.viewProjection[0][0];
        perFrameData.worldFrustumPlanes[0].y = perFrameData.viewProjection[1][3] + perFrameData.viewProjection[1][0];
        perFrameData.worldFrustumPlanes[0].z = perFrameData.viewProjection[2][3] + perFrameData.viewProjection[2][0];
        perFrameData.worldFrustumPlanes[0].w = perFrameData.viewProjection[3][3] + perFrameData.viewProjection[3][0];
        // right
        perFrameData.worldFrustumPlanes[1].x = perFrameData.viewProjection[0][3] - perFrameData.viewProjection[0][0];
        perFrameData.worldFrustumPlanes[1].y = perFrameData.viewProjection[1][3] - perFrameData.viewProjection[1][0];
        perFrameData.worldFrustumPlanes[1].z = perFrameData.viewProjection[2][3] - perFrameData.viewProjection[2][0];
        perFrameData.worldFrustumPlanes[1].w = perFrameData.viewProjection[3][3] - perFrameData.viewProjection[3][0];
        // bottom
        perFrameData.worldFrustumPlanes[2].x = perFrameData.viewProjection[0][3] + perFrameData.viewProjection[0][1];
        perFrameData.worldFrustumPlanes[2].y = perFrameData.viewProjection[1][3] + perFrameData.viewProjection[1][1];
        perFrameData.worldFrustumPlanes[2].z = perFrameData.viewProjection[2][3] + perFrameData.viewProjection[2][1];
        perFrameData.worldFrustumPlanes[2].w = perFrameData.viewProjection[3][3] + perFrameData.viewProjection[3][1];
        // top
        perFrameData.worldFrustumPlanes[3].x = perFrameData.viewProjection[0][3] - perFrameData.viewProjection[0][1];
        perFrameData.worldFrustumPlanes[3].y = perFrameData.viewProjection[1][3] - perFrameData.viewProjection[1][1];
        perFrameData.worldFrustumPlanes[3].z = perFrameData.viewProjection[2][3] - perFrameData.viewProjection[2][1];
        perFrameData.worldFrustumPlanes[3].w = perFrameData.viewProjection[3][3] - perFrameData.viewProjection[3][1];
        // near
        perFrameData.worldFrustumPlanes[4].x = perFrameData.viewProjection[0][2];
        perFrameData.worldFrustumPlanes[4].y = perFrameData.viewProjection[1][2];
        perFrameData.worldFrustumPlanes[4].z = perFrameData.viewProjection[2][2];
        perFrameData.worldFrustumPlanes[4].w = perFrameData.viewProjection[3][2];
        // far
        perFrameData.worldFrustumPlanes[5].x = perFrameData.viewProjection[0][3] - perFrameData.viewProjection[0][2];
        perFrameData.worldFrustumPlanes[5].y = perFrameData.viewProjection[1][3] - perFrameData.viewProjection[1][2];
        perFrameData.worldFrustumPlanes[5].z = perFrameData.viewProjection[2][3] - perFrameData.viewProjection[2][2];
        perFrameData.worldFrustumPlanes[5].w = perFrameData.viewProjection[3][3] - perFrameData.viewProjection[3][2];

        // Normalize the plane equations.
        for (int i = 0; i < 6; ++i) {
            planeNormalize(perFrameData.worldFrustumPlanes[i]);
        }

        perFrameData.ambientLight = glm::vec4(mAmbientLight, 1.0f);
        perFrameData.useBlinnPhong = mUseBlinnPhong;
        perFrameData.useGammaCorrection = mUseGammaCorrection;
        perFrameData.gamma = mGamma;
        mPerFrameBuffer->writeData(&perFrameData, sizeof(perFrameData));
    }

    {
        TerrainSetting ts{};
        ts.tessFactor[0]       = 64;
        ts.tessFactor[1]       = 64;
        ts.tessFactor[2]       = 64;
        ts.tessFactor[3]       = 64;
        ts.insideTessFactor[0] = 64;
        ts.insideTessFactor[1] = 64;
        ts.maxTess = 6;
        ts.minTess = 0;
        ts.maxDistance = 1000.0f;
        ts.minDistance = 20.0f;
        mTerrainSettings->writeData(&ts, sizeof(ts));
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

    // Render Mesh Normals
    {
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mDrawMeshNormals.pipeline->getPipeline());
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            mDrawMeshNormals.pipeline->getPipelineLayout(),
            0 /*firstSet*/,
            1 /*nbSet*/,
            &mDrawMeshNormals.descriptorSet,
            0,
            nullptr
        );

        struct {
            glm::mat4 transform;
            glm::mat4 normalMatrix;
        }aabb;
        auto view           = mRegistry->view<CTransform, CMesh>();
        for (auto [entity, ctrans, cmesh] : view.each()) {
            const auto translateMat  = glm::translate(glm::mat4(1), ctrans.position);
            const auto rotationMat   = glm::eulerAngleYXZ(glm::radians(ctrans.rotation.y), glm::radians(ctrans.rotation.x), glm::radians(ctrans.rotation.z));
            const auto scaleMat      = glm::scale(glm::mat4(1), ctrans.scale);
            aabb.transform     = translateMat * rotationMat * scaleMat;
            aabb.normalMatrix  = glm::transpose(glm::inverse(aabb.transform));
            vkCmdPushConstants(
                cmd,
                mDrawMeshNormals.pipeline->getPipelineLayout(),
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(aabb),
                reinterpret_cast<void*>(&aabb)
            );

            Renderer::DrawMesh(cmd, cmesh.mesh);
        }
    }

    // Render Terrain
    {
        //vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mDrawTerrain.pipeline->getPipeline());
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            mDrawTerrain.pipeline->getPipelineLayout(),
            0 /*firstSet*/,
            1 /*nbSet*/,
            &mDrawTerrain.descriptorSet0,
            0,
            nullptr
        );

        if(mTerrainVisible) {
            auto view           = mRegistry->view<CTransform, CTerrain>();
            for (auto [entity, ctrans, cterrain] : view.each()) {
                if(!mDrawTerrain.descriptorSet1) {
                    mDrawTerrain.descriptorSet1 = mDescriptorPool.allocate(mDrawTerrain.pipeline->getDescriptorSetLayouts()[1]);
                    VulkanContext::setDebugObjectName((uint64_t)mDrawTerrain.descriptorSet0, VK_OBJECT_TYPE_DESCRIPTOR_SET, "TerrainDescriptorSet1" );
                    VulkanContext::setDebugObjectName((uint64_t)mDrawTerrain.pipeline->getDescriptorSetLayouts()[1], VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "TerrainDescriptorSetLayout1" );

                    VkDescriptorImageInfo descriptorImageInfo;
                    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                    descriptorImageInfo.imageView   = cterrain.terrain->getHeightMap()->getImageView();
                    descriptorImageInfo.sampler     = cterrain.terrain->getHeightMap()->getSampler();

                    VkWriteDescriptorSet writeDescriptorSet{};
                    writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writeDescriptorSet.dstSet          = mDrawTerrain.descriptorSet1;
                    writeDescriptorSet.dstBinding      = 0;
                    writeDescriptorSet.descriptorCount = 1;
                    writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    writeDescriptorSet.pImageInfo      = &descriptorImageInfo;
                    vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);

                    // buffers
                    {
                        VkDescriptorBufferInfo descriptorInfo;
                        descriptorInfo.buffer = mTerrainSettings->getBuffer();
                        descriptorInfo.offset = 0;
                        descriptorInfo.range  = VK_WHOLE_SIZE;

                        writeDescriptorSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        writeDescriptorSet.dstSet          = mDrawTerrain.descriptorSet1;
                        writeDescriptorSet.dstBinding      = 1;
                        writeDescriptorSet.descriptorCount = 1;
                        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        writeDescriptorSet.pBufferInfo      = &descriptorInfo;
                        vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);
                    }

                    // diffuse map
                    {
                        VkDescriptorImageInfo imageInfo[5]{};
                        imageInfo[0].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[0].imageView      = cterrain.diffuseMap0->getImageView();
                        imageInfo[0].sampler        = cterrain.diffuseMap0->getSampler();
                        imageInfo[1].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[1].imageView      = cterrain.diffuseMap1->getImageView();
                        imageInfo[1].sampler        = cterrain.diffuseMap1->getSampler();
                        imageInfo[2].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[2].imageView      = cterrain.diffuseMap2->getImageView();
                        imageInfo[2].sampler        = cterrain.diffuseMap2->getSampler();
                        imageInfo[3].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[3].imageView      = cterrain.diffuseMap3->getImageView();
                        imageInfo[3].sampler        = cterrain.diffuseMap3->getSampler();
                        imageInfo[4].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[4].imageView      = cterrain.diffuseMap4->getImageView();
                        imageInfo[4].sampler        = cterrain.diffuseMap4->getSampler();
                        writeDescriptorSet.dstSet          = mDrawTerrain.descriptorSet1;
                        writeDescriptorSet.dstBinding      = 2;
                        writeDescriptorSet.descriptorCount = 5;
                        writeDescriptorSet.dstArrayElement = 0;
                        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        writeDescriptorSet.pImageInfo      = imageInfo;
                        vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);
                    }
                    // normal map
                    {
                        VkDescriptorImageInfo imageInfo[5]{};
                        imageInfo[0].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[0].imageView      = cterrain.normalMap0->getImageView();
                        imageInfo[0].sampler        = cterrain.normalMap0->getSampler();
                        imageInfo[1].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[1].imageView      = cterrain.normalMap1->getImageView();
                        imageInfo[1].sampler        = cterrain.normalMap1->getSampler();
                        imageInfo[2].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[2].imageView      = cterrain.normalMap2->getImageView();
                        imageInfo[2].sampler        = cterrain.normalMap2->getSampler();
                        imageInfo[3].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[3].imageView      = cterrain.normalMap3->getImageView();
                        imageInfo[3].sampler        = cterrain.normalMap3->getSampler();
                        imageInfo[4].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[4].imageView      = cterrain.normalMap4->getImageView();
                        imageInfo[4].sampler        = cterrain.normalMap4->getSampler();
                        writeDescriptorSet.dstSet          = mDrawTerrain.descriptorSet1;
                        writeDescriptorSet.dstBinding      = 3;
                        writeDescriptorSet.descriptorCount = 5;
                        writeDescriptorSet.dstArrayElement = 0;
                        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        writeDescriptorSet.pImageInfo      = imageInfo;
                        vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);
                    }
                    // specular map
                    {
                        VkDescriptorImageInfo imageInfo[5]{};
                        imageInfo[0].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[0].imageView      = cterrain.specularMap0->getImageView();
                        imageInfo[0].sampler        = cterrain.specularMap0->getSampler();
                        imageInfo[1].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[1].imageView      = cterrain.specularMap1->getImageView();
                        imageInfo[1].sampler        = cterrain.specularMap1->getSampler();
                        imageInfo[2].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[2].imageView      = cterrain.specularMap2->getImageView();
                        imageInfo[2].sampler        = cterrain.specularMap2->getSampler();
                        imageInfo[3].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[3].imageView      = cterrain.specularMap3->getImageView();
                        imageInfo[3].sampler        = cterrain.specularMap3->getSampler();
                        imageInfo[4].imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                        imageInfo[4].imageView      = cterrain.specularMap4->getImageView();
                        imageInfo[4].sampler        = cterrain.specularMap4->getSampler();
                        writeDescriptorSet.dstSet          = mDrawTerrain.descriptorSet1;
                        writeDescriptorSet.dstBinding      = 4;
                        writeDescriptorSet.descriptorCount = 5;
                        writeDescriptorSet.dstArrayElement = 0;
                        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        writeDescriptorSet.pImageInfo      = imageInfo;
                        vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);
                    }

                    // blend map
                    descriptorImageInfo.imageLayout    = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
                    descriptorImageInfo.imageView      = cterrain.blendMap->getImageView();
                    descriptorImageInfo.sampler        = cterrain.blendMap->getSampler();
                    writeDescriptorSet.dstSet          = mDrawTerrain.descriptorSet1;
                    writeDescriptorSet.dstBinding      = 5;
                    writeDescriptorSet.descriptorCount = 1;
                    writeDescriptorSet.dstArrayElement = 0;
                    writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    writeDescriptorSet.pImageInfo      = &descriptorImageInfo;
                    vkUpdateDescriptorSets(VulkanContext::getDevice(), 1, &writeDescriptorSet, 0, nullptr);
                }
                vkCmdBindDescriptorSets(
                    cmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    mDrawTerrain.pipeline->getPipelineLayout(),
                    1 /*firstSet*/,
                    1 /*nbSet*/,
                    &mDrawTerrain.descriptorSet1,
                    0,
                    nullptr
                );

                VkDeviceSize offset{};
                VkBuffer buffer = cterrain.terrain->getVertexBuffer()->getBuffer();
                vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
                vkCmdBindIndexBuffer(cmd, cterrain.terrain->getIndexBuffer()->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd, cterrain.terrain->getNumIndices(), 1, 0, 0, 1);

                if(mTerrainAABBVisible) {
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
                    for(unsigned i = 0; i < 1024; i++) {
                        aabb.transform = glm::mat4(1);
                        if(i % 2) {
                            aabb.color   = {1.f, 1.f, 1.f};
                        } else {
                            aabb.color   = {0.f, 0.f, 1.f};
                        }

                        cterrain.terrain->getBound(i, aabb.min, aabb.max);
                        vkCmdPushConstants(cmd, mDrawMeshAABB.pipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(aabb), reinterpret_cast<void*>(&aabb));
                        vkCmdDraw(cmd, 1, 1, 0, 0);
                    }
                }
            }
        }
    }
}
