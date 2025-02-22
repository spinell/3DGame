#include "Renderer.h"

void Renderer::Init() {}

void Renderer::Shutdown() {}

void Renderer::DrawMesh(VkCommandBuffer cmd, const Mesh& mesh) {
    VkDeviceSize offset = 0;
    VkBuffer     vBuffer = mesh.vertexBuffer->getBuffer();
    if(mesh.subMeshs.size()) {
#if 0
        for(const auto& subMesh : mesh.subMeshs) {

            offset = subMesh.vertexBufferOffset;
            vkCmdBindVertexBuffers(cmd, 0, 1, &vBuffer, &offset);
            vkCmdBindIndexBuffer(cmd, mesh.indexBuffer->getBuffer(), subMesh.indexBufferOffset, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, subMesh.nbIndices, 1/*intance count*/, 0/*firstIndex*/, 0/*vertexOffset*/, 0/*firstInstance*/);
        }
#else
        vkCmdBindVertexBuffers(cmd, 0, 1, &vBuffer, &offset);
        vkCmdBindIndexBuffer(cmd, mesh.indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        for(const auto& subMesh : mesh.subMeshs) {
            vkCmdDrawIndexed(cmd, subMesh.nbIndices, 1/*intance count*/, subMesh.firstIndex/*firstIndex*/, subMesh.vertexOffset/*vertexOffset*/, 0/*firstInstance*/);
        }
#endif
    }
    else {
        vkCmdBindVertexBuffers(cmd, 0, 1, &vBuffer, &offset);
        vkCmdBindIndexBuffer(cmd, mesh.indexBuffer->getBuffer(), offset, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
    }
}
