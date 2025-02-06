#pragma once
#include <glm/glm.hpp>

#include <vector>

/**
 * \brief
 */
class GeometryGenerator {
private:
public:
    struct UV {
        UV() = default;
        UV(float pU, float pV) : u(pU), v(pV) {}
        float u{};
        float v{};
    };

    struct Vertex {
        Vertex() {}

        Vertex(glm::vec3& pPosition, glm::vec3& pNormal, glm::vec3& pTangentU, UV& pUV)
            : Position(pPosition), Normal(pNormal), TangentU(pTangentU), TexC(pUV) {}

        Vertex(float pX,
               float pY,
               float pZ,
               float pNormalx,
               float pNormaly,
               float pNormalz,
               float pTangentX,
               float pTangentY,
               float pTangentZ,
               float pU,
               float pV)
            : Position(pX, pY, pZ),
              Normal(pNormalx, pNormaly, pNormalz),
              TangentU(pTangentX, pTangentY, pTangentZ),
              TexC(pU, pV) {}

        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 TangentU;
        UV        TexC;
    };

    struct MeshData {
        std::vector<Vertex>       Vertices;
        std::vector<unsigned int> Indices;
    };

public:
    GeometryGenerator() {}
    ~GeometryGenerator() {}
    void createBox(float pWidth, float pHeight, float pDepth, MeshData& pMeshData);
    void createGrid(float        pGridWidth,
                    float        pGridDepth,
                    unsigned int pNbVertexWidth,
                    unsigned int pNbVertexDepth,
                    MeshData&    pMeshData);
    void createGeoSphere(float pRadius, unsigned int pSubdivisionCount, MeshData& pMeshData);
    void createCylinder(float        pBottomRadius,
                        float        pTopRadius,
                        float        pHeight,
                        unsigned int pSliceCount,
                        unsigned int pStackCount,
                        MeshData&    pMeshData);
    void createSphere(float        pRadius,
                      unsigned int pSliceCount,
                      unsigned int pStackCount,
                      MeshData&    meshData);

    void createFullscreenQuad(MeshData& meshData);

private:
    void _subdivide(MeshData& meshData);
    void _buildCylinderTopCap(float        topRadius,
                              float        height,
                              unsigned int sliceCount,
                              MeshData&    meshData);
    void _buildCylinderBottomCap(float        bottomRadius,
                                 float        height,
                                 unsigned int sliceCount,
                                 MeshData&    meshData);
};
