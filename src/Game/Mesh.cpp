#include "Mesh.h"

#include "GeometryGenerator.h"

namespace {
    void uploadData(const GeometryGenerator::MeshData& meshData, Mesh& mesh) {
        const auto vertexSize = meshData.Vertices.size() * sizeof(GeometryGenerator::Vertex);
        const auto indexSize  = meshData.Indices.size() * sizeof(unsigned);


        mesh.indexCount   = meshData.Indices.size();
        mesh.vertexBuffer = VulkanContext::createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexSize,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        mesh.indexBuffer  = VulkanContext::createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexSize,
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        void* pData{};
        vmaMapMemory(VulkanContext::getVmaAllocator(), mesh.vertexBuffer.allocation, &pData);
        std::memcpy(pData, meshData.Vertices.data(), vertexSize);
        vmaUnmapMemory(VulkanContext::getVmaAllocator(), mesh.vertexBuffer.allocation);

        // upload index data
        vmaMapMemory(VulkanContext::getVmaAllocator(), mesh.indexBuffer.allocation, &pData);
        std::memcpy(pData, meshData.Indices.data(), indexSize);
        vmaUnmapMemory(VulkanContext::getVmaAllocator(), mesh.indexBuffer.allocation);
    }
};

Mesh Mesh::CreateMeshCube(float size) { return CreateMeshCube(size, size, size); }

Mesh Mesh::CreateMeshCube(float width, float height, float depth) {
    GeometryGenerator           geometryGenerator;
    GeometryGenerator::MeshData meshData;
    geometryGenerator.createBox(width, height, depth, meshData);

    // upload vertex data
    Mesh mesh;
    uploadData(meshData, mesh);

    return mesh;
}

Mesh Mesh::CreateGrid(float        width,
                      float        depth,
                      unsigned int nbVertexWidth,
                      unsigned int nVertexDepth) {
    GeometryGenerator           geometryGenerator;
    GeometryGenerator::MeshData meshData;
    geometryGenerator.createGrid(width, depth, nbVertexWidth, nVertexDepth, meshData);

    // upload vertex data
    Mesh mesh;
    uploadData(meshData, mesh);

    return mesh;
}

Mesh Mesh::CreateGeoSphere(float radius, unsigned int subdivisionCount) {
    GeometryGenerator           geometryGenerator;
    GeometryGenerator::MeshData meshData;
    geometryGenerator.createGeoSphere(radius, subdivisionCount, meshData);

    // upload vertex data
    Mesh mesh;
    uploadData(meshData, mesh);

    return mesh;
}

Mesh Mesh::CreateCylinder(float        bottomRadius,
                          float        topRadius,
                          float        height,
                          unsigned int sliceCount,
                          unsigned int stackCount) {
    GeometryGenerator           geometryGenerator;
    GeometryGenerator::MeshData meshData;
    geometryGenerator.createCylinder(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

    // upload vertex data
    Mesh mesh;
    uploadData(meshData, mesh);

    return mesh;
}

Mesh Mesh::CreateSphere(float radius, unsigned int sliceCount, unsigned int stackCount) {
    GeometryGenerator           geometryGenerator;
    GeometryGenerator::MeshData meshData;
    geometryGenerator.createSphere(radius, sliceCount, stackCount, meshData);

    // upload vertex data
    Mesh mesh;
    uploadData(meshData, mesh);

    return mesh;
}
