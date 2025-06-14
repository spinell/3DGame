#include "Terrain.h"

#include "stb_image.h"

#include <expected>
#include <fstream>

Terrain::Terrain() {
    loadHeightFromFile("G:/workspace/FPSGame/fpsgame/sources/FPSGame3/data/terrains/terrain.png");
    //loadHeightFromFile("G:/terrain.raw");

    // Divide heightmap into patches such that each patch has CellsPerPatch.
    mNumPatchPerRows = ((mHeightMapHeight - 1) / CELL_PER_PATCH) + 1;
    mNumPatchPerCols = ((mHeightMapWidth - 1) / CELL_PER_PATCH) + 1;

    mNumPatchVertices  = mNumPatchPerRows * mNumPatchPerCols;
    mNumPatchQuadFaces = (mNumPatchPerRows - 1) * (mNumPatchPerCols - 1);

    calcAllPathBoundY();
    buildQuadPatchVertex();
    buildQuadPatchIndex();

    {
        VulkanBufferCreateInfo createInfo{};
        createInfo.name           = "TerrainVB";
        createInfo.sizeInByte     = mPatchVertices.size() * sizeof(Vertex);
        createInfo.usage          = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        createInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mVertexBuffer             = VulkanBuffer::Create(createInfo);
        mVertexBuffer->writeData(mPatchVertices.data(), createInfo.sizeInByte);

        createInfo.name           = "TerrainIB";
        createInfo.sizeInByte     = mIndices.size() * sizeof(unsigned);
        createInfo.usage          = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        createInfo.memoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        mIndexBuffer              = VulkanBuffer::Create(createInfo);
        mIndexBuffer->writeData(mIndices.data(), createInfo.sizeInByte);
    }
    {
        VulkanTexture2DCreateInfo createInfo{};
        createInfo.name   = "TerrainHeightMap";
        createInfo.width  = 2049;
        createInfo.height = 2049;
        createInfo.format = VK_FORMAT_R32_SFLOAT;
        mHeightMapTexture = VulkanTexture::Create(createInfo, mHeightMap.data());
    }
}

Terrain::~Terrain() {}

void Terrain::loadHeightFromFile(const std::filesystem::path& path) {
    int   width, height, channels;
    auto* data = stbi_load(path.string().c_str(), &width, &height, &channels, 1);
    if (data) {
        mHeightMapRaw.resize(mHeightMapWidth * mHeightMapHeight);
        mHeightMap.resize(mHeightMapWidth * mHeightMapHeight);

        unsigned j = 0;
        for (std::size_t i = 0; i < width * height; i++) {
            mHeightMapRaw[j] = data[i];
            mHeightMap[j]    = data[i] / 255.0 * mHeightMapScale;
            j++;
        }
        stbi_image_free(data);
    }
}

float Terrain::getHeight(float x, float z) const {
    // check out of bound
    assert((x >= 0.5f * -getWidth()) && (x <= 0.5f * getWidth()));
    assert((z >= 0.5f * -getDepth()) && (z <= 0.5f * getDepth()));

    // Transform from terrain local space to "cell" space.
    const float c = (x + 0.5f * getWidth()) / mCellSpacing;
    const float d = (z - 0.5f * getDepth()) / -mCellSpacing;

    // Get the row and column we are in.
    const int row = (int)floorf(d);
    const int col = (int)floorf(c);

    // Grab the heights of the cell we are in.
    // A*--*B
    //  | /|
    //  |/ |
    // C*--*D
    const float A = mHeightMap[row * mHeightMapWidth + col];
    const float B = mHeightMap[row * mHeightMapWidth + col + 1];
    const float C = mHeightMap[(row + 1) * mHeightMapWidth + col];
    const float D = mHeightMap[(row + 1) * mHeightMapWidth + col + 1];

    // Where we are relative to the cell.
    const float s = c - (float)col;
    const float t = d - (float)row;

    // If upper triangle ABC.
    if (s + t <= 1.0f) {
        const float uy = B - A;
        const float vy = C - A;
        return A + s * uy + t * vy;
    }
    // lower triangle DCB.
    else {
        const float uy = C - D;
        const float vy = B - D;
        return D + (1.0f - s) * uy + (1.0f - t) * vy;
    }
}

void Terrain::calcAllPathBoundY() {
    mPatchBoundsY.resize(mNumPatchQuadFaces);
    for(unsigned i = 0; i < mNumPatchPerRows - 1; i++) {
        for(unsigned y = 0; y < mNumPatchPerCols - 1; y++) {
            calcPathBoundY(i, y);
        }
    }
}

void Terrain::calcPathBoundY(unsigned i, unsigned j) {
    // scan the heightmap values this patch covers and
    // compute the min/max height.
    const unsigned x0 = CELL_PER_PATCH * j;
    const unsigned x1 = CELL_PER_PATCH * (j + 1);
    const unsigned y0 = CELL_PER_PATCH * i;
    const unsigned y1 = CELL_PER_PATCH * (i + 1);

    float minY =  std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    for(unsigned y = y0; y <= y1; y++) {
        for(unsigned x = x0; x <= x1; x++) {
            const unsigned k = y * mHeightMapWidth + x;
            minY = std::min(minY, mHeightMap[k]);
            maxY = std::max(maxY, mHeightMap[k]);
        }
    }

    const unsigned patchID = i * (mNumPatchPerCols-1) + j;
    mPatchBoundsY[patchID] = {minY, maxY};
}

void Terrain::buildQuadPatchVertex() {
    mPatchVertices.resize(mNumPatchPerRows * mNumPatchPerCols);

    const float halfWidth  = 0.5f * getWidth();
    const float halfDepth  = 0.5f * getDepth();
    const float patchWidth = getWidth() / (mNumPatchPerCols - 1);
    const float patchDepth = getDepth() / (mNumPatchPerRows - 1);
    const float du         = 1.0f / (mNumPatchPerCols - 1);
    const float dv         = 1.0f / (mNumPatchPerRows - 1);

    for (unsigned int i = 0; i < mNumPatchPerRows; ++i) {
        const float z = halfDepth - i * patchDepth;
        for (unsigned int j = 0; j < mNumPatchPerCols; ++j) {
            const float x = -halfWidth + j * patchWidth;

            mPatchVertices[i * mNumPatchPerCols + j].pos = glm::vec3(x, 0.0f, z);

            // Stretch texture over grid.
            mPatchVertices[i * mNumPatchPerCols + j].tex.x = j * du;
            mPatchVertices[i * mNumPatchPerCols + j].tex.y = i * dv;
        }
    }

    // Store axis-aligned bounding box y-bounds in upper-left patch corner.
    for (unsigned int i = 0; i < mNumPatchPerRows - 1; ++i) {
        for (unsigned int j = 0; j < mNumPatchPerCols - 1; ++j) {
            unsigned int patchID                              = i * (mNumPatchPerCols - 1) + j;
            mPatchVertices[i * mNumPatchPerCols + j].boundsY = mPatchBoundsY[patchID];
        }
    }
}

void Terrain::buildQuadPatchIndex() {
    mIndices.resize(mNumPatchQuadFaces * 4); // 4 indices per quad face

    // Iterate over each quad and compute indices.
    int k = 0;
    for (unsigned int i = 0; i < mNumPatchPerRows - 1; ++i) {
        for (unsigned int j = 0; j < mNumPatchPerCols - 1; ++j) {
            // Top row of 2x2 quad patch
            mIndices[k]     = i * mNumPatchPerCols + j;
            mIndices[k + 1] = i * mNumPatchPerCols + j + 1;

            // Bottom row of 2x2 quad patch
            mIndices[k + 2] = (i + 1) * mNumPatchPerCols + j;
            mIndices[k + 3] = (i + 1) * mNumPatchPerCols + j + 1;

            k += 4; // next quad
        }
    }
}
