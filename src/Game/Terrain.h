#pragma once
#include "vulkan/VulkanBuffer.h"
#include "vulkan/VulkanTexture.h"

#include <glm/glm.hpp>

#include <filesystem>

class TerrainLayer {
public:
    TerrainLayer() = default;
    ~TerrainLayer() = default;

private:
    VulkanTexturePtr diffuseMap;
    VulkanTexturePtr normalMap;
    VulkanTexturePtr specularMap;
    float            normalScale = 1.0f; // ???
    float            tillingX    = 1.0f;
    float            tillingY    = 1.0f;
    float            offsetX     = 0.0f;
    float            offsetY     = 0.0f;
    float            rotationY   = 0.0f;
};

class Terrain {
public:
    struct Vertex {
        glm::vec3 pos;
        glm::vec2 tex;
        glm::vec2 boundsY;
    };

    Terrain();
    ~Terrain();

    void loadHeightFromFile(const std::filesystem::path& path);

    /// @brief
    /// @return
    float getDepth() const { return (mHeightMapHeight - 1) * mCellSpacing; };

    /// @brief
    /// @return
    float getWidth() const { return (mHeightMapWidth - 1) * mCellSpacing; };

    /// @brief Return the height of the terrain at a specific (x,z) position.
    /// @param x The x position from which the height will be retrived.
    /// @param z The z position from which the height will be retrived.
    /// @return The height of the given point (x,z).
    ///
    /// @note x must be in range [-getWidth() / 2, getWidth() / 2]
    /// @note z must be in range [-getDepth() / 2, getDepth() / 2]
    float getHeight(float x, float z) const;

    VulkanBufferPtr  getVertexBuffer() const { return mVertexBuffer; }
    VulkanBufferPtr  getIndexBuffer() const { return mIndexBuffer; }
    VulkanTexturePtr getHeightMap() const { return mHeightMapTexture; }
    std::size_t      getNumIndices() const { return mIndices.size(); }

    void getBound(unsigned patchID, glm::vec3& min, glm::vec3& max) const {
        const auto    i0 = mIndices[patchID * 4];
        const auto    i1 = mIndices[patchID * 4 + 1];
        const auto    i2 = mIndices[patchID * 4 + 2];
        const auto    i3 = mIndices[patchID * 4 + 3];
        const Vertex& v0 = mPatchVertices[i0];
        const Vertex& v1 = mPatchVertices[i1];
        const Vertex& v2 = mPatchVertices[i2];
        const Vertex& v3 = mPatchVertices[i3];
        min              = {v0.pos.x, mPatchBoundsY[patchID].x, v3.pos.z};
        max              = {v1.pos.x, mPatchBoundsY[patchID].y, v0.pos.z};
        return;
#if 0
        const unsigned col = patchID / mNumPatchPerCols;
        const Vertex& v0 = mPatchVertices[patchID];                                     // top left
        const Vertex& v1 = mPatchVertices[patchID + 1];                                 // top right
        const Vertex& v3 = mPatchVertices[col * mNumPatchPerRows + (col * -1)];    // bottom right
        min = {v0.pos.x, mPatchBoundsY[patchID].x, v3.pos.z};
        max = {v1.pos.x, mPatchBoundsY[patchID].y, v0.pos.z};
#endif
    }

    void addLayers(const TerrainLayer& layer) { mlayers.push_back(layer); }
    const std::vector<TerrainLayer>& getLayers() const { return mlayers; }

private:
    void calcAllPathBoundY();
    void calcPathBoundY(unsigned i, unsigned j);
    void buildQuadPatchVertex();
    void buildQuadPatchIndex();

    static const int CELL_PER_PATCH = 64;
    unsigned int     mNumPatchVertices;
    unsigned int     mNumPatchQuadFaces;

    /// @brief Number of patch per row.
    unsigned int mNumPatchPerRows;

    /// @brief Number of patch per column.
    unsigned int mNumPatchPerCols;

    unsigned int               mHeightMapHeight = 2049;
    unsigned int               mHeightMapWidth  = 2049;
    float                      mHeightMapScale  = 50.f;
    float                      mCellSpacing     = 1.0f;
    std::vector<float>         mHeightMap;
    std::vector<glm::vec2>     mPatchBoundsY;
    std::vector<unsigned char> mHeightMapRaw;

    /// @brief Terrain grid vertices (control point)
    std::vector<Vertex> mPatchVertices;

    /// @brief Terrain grid indices
    std::vector<unsigned int> mIndices;

    VulkanBufferPtr           mVertexBuffer{};
    VulkanBufferPtr           mIndexBuffer{};
    VulkanTexturePtr          mHeightMapTexture{};
    std::vector<TerrainLayer> mlayers;
};
