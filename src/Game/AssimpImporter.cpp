#include "AssimpImporter.h"

#include <Engine/Log.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

struct Vertex {
    float position[3];
    float normal[3];
    float tangent[3];
    float uv[2];
};

namespace {
/// @brief Print datain the scene (For debugging only).
void printMeshs(const aiScene* scene) {
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];
        ENGINE_CORE_INFO("Mesh Name:                 {}", mesh->mName.C_Str());
        ENGINE_CORE_INFO(" HasBones:                 {}", mesh->HasBones());
        ENGINE_CORE_INFO(" HasFaces:                 {}", mesh->HasFaces());
        ENGINE_CORE_INFO(" HasNormals:               {}", mesh->HasNormals());
        ENGINE_CORE_INFO(" HasPositions:             {}", mesh->HasPositions());
        ENGINE_CORE_INFO(" HasTangentsAndBitangents: {}", mesh->HasTangentsAndBitangents());
        ENGINE_CORE_INFO(" NumUVChannels:            {}", mesh->GetNumUVChannels());
        ENGINE_CORE_INFO(" NumColorChannels:         {}", mesh->GetNumColorChannels());
        ENGINE_CORE_INFO(" PrimitiveTypes:           {}", mesh->mPrimitiveTypes);
        ENGINE_CORE_INFO(" AABB:                     [{},{},{}] [{},{},{}]", mesh->mAABB.mMin.x,
                         mesh->mAABB.mMin.y, mesh->mAABB.mMin.z, mesh->mAABB.mMax.x,
                         mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);
        ENGINE_CORE_INFO(" NumVertices:              {}", mesh->mNumVertices);
        ENGINE_CORE_INFO(" NumFaces:                 {}", mesh->mNumFaces);
        ENGINE_CORE_INFO(" NumBones:                 {}", mesh->mNumBones);
        ENGINE_CORE_INFO(" NumAnimMeshes:            {}", mesh->mNumAnimMeshes);
        ENGINE_CORE_INFO(" MaterialIndex:            {}", mesh->mMaterialIndex);

        for (unsigned int boneIdx = 0; boneIdx < mesh->mNumBones; boneIdx++) {
            const aiBone* bone = mesh->mBones[boneIdx];
            ENGINE_CORE_INFO("   Bone #{}: {}", boneIdx, bone->mName.C_Str());
            ENGINE_CORE_INFO("    -NumWeights: {}", bone->mNumWeights);
            if (bone->mNode) {
                ENGINE_CORE_INFO("    -NodeName:   {}", bone->mNode->mName.C_Str());
            }
            if (bone->mArmature) {
                ENGINE_CORE_INFO("    -Armature:   {}", bone->mArmature->mName.C_Str());
            }
        }
    }
}

/// @brief Print node hierachy (For debugging only).
void printNodes(const aiNode* node, int depth = 0) {
    aiVector3D translation;
    aiVector3D scaling;
    aiVector3D rotation;
    node->mTransformation.Decompose(scaling, rotation, translation);
    auto tranStr  = std::format("{},{},{}", translation.x, translation.y, translation.z);
    auto rotStr   = std::format("{},{},{}", rotation.x, rotation.y, rotation.z);
    auto scaleStr = std::format("{},{},{}", scaling.x, scaling.y, scaling.z);
    ENGINE_CORE_INFO("{:{}}{} [{}] [{}] [{}]", "", depth, node->mName.C_Str(), tranStr, rotStr,
                     scaleStr);
    for (unsigned i = 0; i < node->mNumMeshes; i++) {
        ENGINE_CORE_INFO("{:{}} Mesh index #{}", "", depth, node->mMeshes[i]);
    }

    ++depth;
    for (unsigned i = 0; i < node->mNumChildren; i++) {
        printNodes(node->mChildren[i], depth);
    }
}

/// @brief Print material in the scene (For debugging only).
void printMaterials(const aiScene* scene) {
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        ENGINE_CORE_INFO(" Material #{} Name: {} ", i, material->GetName().C_Str());
        for (unsigned int propIdx = 0; propIdx < material->mNumProperties; propIdx++) {
            ENGINE_CORE_INFO("  Property #{} {}", propIdx,
                             material->mProperties[propIdx]->mKey.C_Str());
        }

        aiColor3D ambientColor{};
        if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  AmbientColor: {},{},{}", ambientColor.r, ambientColor.g,
                             ambientColor.b);
        };
        aiColor3D diffuseColor{};
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  DiffuseColor: {},{},{}", diffuseColor.r, diffuseColor.g,
                             diffuseColor.b);
        };
        aiColor3D specularColor{};
        if (material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  SpecularColor: {},{},{}", specularColor.r, specularColor.g,
                             specularColor.b);
        };
        aiColor3D emissiveColor{};
        if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  EmissiveColor: {},{},{}", emissiveColor.r, emissiveColor.g,
                             emissiveColor.b);
        };
        int wireframe{};
        if (material->Get(AI_MATKEY_ENABLE_WIREFRAME, wireframe) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  Wireframe: {}", wireframe);
        };
        int twoside{};
        if (material->Get(AI_MATKEY_TWOSIDED, twoside) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  Twoside: {}", twoside);
        };
        int shadingModel{};
        if (material->Get(AI_MATKEY_SHADING_MODEL, shadingModel) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  ShadingModel: {}", shadingModel);
        };
        // Defines the shininess of a phong-shaded material.
        // This is actually the exponent of the phong specular equation
        float shininess{};
        if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  Shininess: {}", shininess);
        };
        // Scales the specular color of the material.
        float shininessStrenght{};
        if (material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrenght) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  ShininessStrenght: {}", shininessStrenght);
        };
        float glossinessFactort{};
        if (material->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossinessFactort) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  GlossinessFactort: {}", glossinessFactort);
        };
        aiString diffuseTexturePath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  DiffuseTexturePath: {}", diffuseTexturePath.C_Str());
        }
        aiString normalTexturePath;
        if (material->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  NormalTexturePath: {}", normalTexturePath.C_Str());
        }
        aiString specularTexturePath;
        if (material->GetTexture(aiTextureType_SPECULAR, 0, &specularTexturePath) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  specularTexturePath: {}", specularTexturePath.C_Str());
        }
        aiString emissiveTexturePath;
        if (material->GetTexture(aiTextureType_EMISSIVE, 0, &emissiveTexturePath) == AI_SUCCESS) {
            ENGINE_CORE_INFO("  emissiveTexturePath: {}", emissiveTexturePath.C_Str());
        }
    }
}

}; // namespace

inline glm::vec3 toGLM(aiVector3D v) { return {v.x, v.y, v.z}; }

AssimpImporter::AssimpImporter() {}

Mesh AssimpImporter::importMesh(const std::filesystem::path& path) {
    Mesh importedMesh;

    ENGINE_CORE_INFO("Importing {}", path.string());

    Assimp::Importer importer;
    // importer.SetPropertyInteger(AI_CONFIG_FBX_CONVERT_TO_M, 1);
    // importer.SetPropertyFloat(AI_CONFIG_FBX_USE_SKELETON_BONE_CONTAINER, .1);
    importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, .01);

    unsigned int importFlags = 0;
    importFlags |= aiProcess_GenNormals;
    importFlags |= aiProcess_CalcTangentSpace;
    importFlags |= aiProcess_Triangulate;
    importFlags |= aiProcess_JoinIdenticalVertices;
    importFlags |= aiProcess_SortByPType;
    importFlags |= aiProcess_GenBoundingBoxes;
    // importFlags |= aiProcess_GenUVCoords;
    importFlags |= aiProcess_FlipUVs;
    importFlags |= aiProcess_LimitBoneWeights;
    // importFlags |= aiProcess_GlobalScale;
    importFlags |= aiProcess_PopulateArmatureData;
    // importFlags |= aiProcess_PreTransformVertices;
    const aiScene* scene = importer.ReadFile(path.string(), importFlags);
    if (!scene) {
        ENGINE_CORE_ERROR("Fail to import {}", path.string());
        ENGINE_CORE_ERROR(" Reason: {}", importer.GetErrorString());
        return importedMesh;
    }

#if ASSIMP_PRINT_INFO
    ENGINE_CORE_INFO("============ Meshes ==============");
    printMeshs(scene);
    ENGINE_CORE_INFO("============ Material ==============");
    printMaterials(scene) ENGINE_CORE_INFO("============ Nodes ==============");
    printNodes(scene->mRootNode);
#endif

    std::vector<Vertex>   vertices;
    std::vector<unsigned> indices;
    unsigned              baseIndex = 0;

    for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++) {
        const aiMesh* aimesh = scene->mMeshes[meshIdx];
        Mesh::SubMesh subMesh;

        subMesh.nbVertices         = aimesh->mNumVertices;
        subMesh.nbIndices          = aimesh->mNumFaces * 3;
        subMesh.vertexBufferOffset = vertices.size() * sizeof(Vertex);
        subMesh.indexBufferOffset  = indices.size() * sizeof(unsigned);
        subMesh.firstIndex         = indices.size();
        subMesh.vertexOffset       = vertices.size();
        subMesh.aabbMin            = toGLM(aimesh->mAABB.mMin);
        subMesh.aabbMax            = toGLM(aimesh->mAABB.mMax);

        for (unsigned int vtxIdx = 0; vtxIdx < aimesh->mNumVertices; vtxIdx++) {
            Vertex vertex;
            vertex.position[0] = aimesh->mVertices[vtxIdx].x;
            vertex.position[1] = aimesh->mVertices[vtxIdx].y;
            vertex.position[2] = aimesh->mVertices[vtxIdx].z;
            vertex.normal[0]   = aimesh->mNormals[vtxIdx].x;
            vertex.normal[1]   = aimesh->mNormals[vtxIdx].y;
            vertex.normal[2]   = aimesh->mNormals[vtxIdx].z;
            vertex.tangent[0]  = aimesh->mTangents[vtxIdx].x;
            vertex.tangent[1]  = aimesh->mTangents[vtxIdx].y;
            vertex.tangent[2]  = aimesh->mTangents[vtxIdx].z;
            if (aimesh->HasTextureCoords(0)) {
                vertex.uv[0] = aimesh->mTextureCoords[0][vtxIdx].x;
                vertex.uv[1] = aimesh->mTextureCoords[0][vtxIdx].y;

                // flip V for vulkan.
                // texture are loaded top row first ....
                // vertex.uv[1] = 1 - vertex.uv[1];
            }

            vertices.push_back(vertex);
        }

        for (unsigned int faceIdx = 0; faceIdx < aimesh->mNumFaces; faceIdx++) {
            const aiFace& face = aimesh->mFaces[faceIdx];
            assert(face.mNumIndices == 3 && "Should be a triangle.");
            for (unsigned int i = 0; i < face.mNumIndices; i++) {
                indices.push_back(face.mIndices[i]);
            }
        }

        importedMesh.subMeshs.push_back(subMesh);
    }

    // Merge submesh AABB
    importedMesh.aabbMin = {FLT_MAX, FLT_MAX, FLT_MAX};
    importedMesh.aabbMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
    for (const auto& submesh : importedMesh.subMeshs) {
        importedMesh.aabbMin.x = std::min(importedMesh.aabbMin.x, submesh.aabbMin.x);
        importedMesh.aabbMin.y = std::min(importedMesh.aabbMin.y, submesh.aabbMin.y);
        importedMesh.aabbMin.z = std::min(importedMesh.aabbMin.z, submesh.aabbMin.z);

        importedMesh.aabbMax.x = std::max(importedMesh.aabbMax.x, submesh.aabbMax.x);
        importedMesh.aabbMax.y = std::max(importedMesh.aabbMax.y, submesh.aabbMax.y);
        importedMesh.aabbMax.z = std::max(importedMesh.aabbMax.z, submesh.aabbMax.z);
    }

    VulkanBufferCreateInfo createInfo{};
    const std::size_t      vertexBufferSize = sizeof(Vertex) * vertices.size();
    createInfo.sizeInByte                   = vertexBufferSize;
    createInfo.name                         = "VB";
    createInfo.usage                        = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    createInfo.memoryProperty =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    importedMesh.vertexBuffer = VulkanBuffer::Create(createInfo);
    importedMesh.vertexBuffer->writeData(vertices.data(),
                                         importedMesh.vertexBuffer->getSizeInByte());

    const std::size_t indexBufferSize = sizeof(unsigned) * indices.size();
    createInfo.name                   = "IB";
    createInfo.sizeInByte             = indexBufferSize;
    createInfo.usage                  = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    createInfo.memoryProperty =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    importedMesh.indexBuffer = VulkanBuffer::Create(createInfo);
    importedMesh.indexBuffer->writeData(indices.data(), importedMesh.indexBuffer->getSizeInByte());
    importedMesh.indexCount = indices.size();

    return importedMesh;
}
