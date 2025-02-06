#include "Renderer.h"

void Renderer::Init() {}

void Renderer::Shutdown() {}

void Renderer::DrawMesh(VkCommandBuffer cmd, const Mesh& mesh) {
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &mesh.vertexBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(cmd, mesh.indexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
}
