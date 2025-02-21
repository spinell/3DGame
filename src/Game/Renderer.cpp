#include "Renderer.h"

void Renderer::Init() {}

void Renderer::Shutdown() {}

void Renderer::DrawMesh(VkCommandBuffer cmd, const Mesh& mesh) {
    VkDeviceSize offset = 0;
    VkBuffer     vBuffer = mesh.vertexBuffer->getBuffer();
    vkCmdBindVertexBuffers(cmd, 0, 1, &vBuffer, &offset);
    vkCmdBindIndexBuffer(cmd, mesh.indexBuffer->getBuffer(), offset, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
}
