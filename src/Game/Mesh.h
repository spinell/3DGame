#pragma once
#include "Vulkan/VulkanBuffer.h"

struct Mesh {
    VulkanBufferPtr   vertexBuffer;
    VulkanBufferPtr   indexBuffer;
    uint32_t indexCount;

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
