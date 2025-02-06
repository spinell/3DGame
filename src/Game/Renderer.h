#pragma once
#include "Mesh.h"
#include "Vulkan/VulkanContext.h"

class Renderer {
public:
    static void Init();
    static void Shutdown();
    static void DrawMesh(VkCommandBuffer cmd, const Mesh& mesh);
};
