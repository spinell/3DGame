#include "Mesh.h"

#include "GeometryGenerator.h"

namespace {
    void uploadData(const GeometryGenerator::MeshData& meshData, Mesh& mesh) {
        const auto vertexSize = meshData.Vertices.size() * sizeof(GeometryGenerator::Vertex);
        const auto indexSize  = meshData.Indices.size() * sizeof(unsigned);


        mesh.indexCount   = meshData.Indices.size();
        VulkanBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sizeInByte     = vertexSize;
        bufferCreateInfo.usage          = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferCreateInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mesh.vertexBuffer = VulkanBuffer::Create(bufferCreateInfo);

        bufferCreateInfo.sizeInByte     = indexSize;
        bufferCreateInfo.usage          = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufferCreateInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mesh.indexBuffer  = VulkanBuffer::Create(bufferCreateInfo);

        mesh.vertexBuffer->writeData(meshData.Vertices.data(), vertexSize);
        mesh.indexBuffer->writeData(meshData.Indices.data(), indexSize);
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
