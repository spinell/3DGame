#pragma once
#include "Vulkan/VulkanContext.h"

struct Mesh {
    Buffer   vertexBuffer;
    Buffer   indexBuffer;
    uint32_t indexCount;

    [[nodiscard]] static Mesh CreateMeshCube(float size);
    [[nodiscard]] static Mesh CreateMeshCube(float width, float height, float depth);
    [[nodiscard]] static Mesh CreateGrid(float        width,
                                         float        depth,
                                         unsigned int nbVertexWidth,
                                         unsigned int nVertexDepth);
};
