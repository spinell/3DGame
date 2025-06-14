#pragma once
#include "Vulkan/VulkanBuffer.h"

#include <glm/glm.hpp>

#include <vector>

struct Mesh {
    struct SubMesh {
        // number of indices in the sub mesh
        unsigned nbIndices;
        // number of vertices in the sub mesh
        unsigned nbVertices;
        // offset in byte in the index buffer
        unsigned indexBufferOffset;
        // offset in byte in the vertex buffer
        unsigned vertexBufferOffset;
        // first index of the submesh in the vertex buffer.
        unsigned firstIndex;
        // Value to add to a index before fetching the vertex.
        unsigned vertexOffset;

        // AABB of the sub mesh
        glm::vec3 aabbMin;
        glm::vec3 aabbMax;
    };
    VulkanBufferPtr      vertexBuffer;
    VulkanBufferPtr      indexBuffer;
    std::vector<SubMesh> subMeshs;
    uint32_t             indexCount;

    // AABB of the mesh
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;

    /// @brief
    /// @param size
    /// @return
    [[nodiscard]] static Mesh CreateMeshCube(float size);

    /// @brief
    /// @param width
    /// @param height
    /// @param depth
    /// @return
    [[nodiscard]] static Mesh CreateMeshCube(float width, float height, float depth);

    /// @brief
    /// @param width
    /// @param depth
    /// @param nbVertexWidth
    /// @param nVertexDepth
    /// @return
    [[nodiscard]] static Mesh CreateGrid(float        width,
                                         float        depth,
                                         unsigned int nbVertexWidth,
                                         unsigned int nVertexDepth);

    /// @brief
    /// @param radius
    /// @param subdivisionCount
    /// @return
    [[nodiscard]] static Mesh CreateGeoSphere(float radius, unsigned int subdivisionCount);

    /// @brief
    /// @param bottomRadius
    /// @param topRadius
    /// @param height
    /// @param sliceCount
    /// @param stackCounta
    /// @return
    [[nodiscard]] static Mesh CreateCylinder(float        bottomRadius,
                                             float        topRadius,
                                             float        height,
                                             unsigned int sliceCount,
                                             unsigned int stackCounta);

    /// @brief
    /// @param radius
    /// @param sliceCount
    /// @param stackCount
    /// @return
    [[nodiscard]] static Mesh CreateSphere(float        radius,
                                           unsigned int sliceCount,
                                           unsigned int stackCount);
};
