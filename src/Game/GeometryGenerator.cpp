#include "GeometryGenerator.h"
#include <numbers>

float AngleFromXY(float x, float y) {
    float theta = 0.0f;

    // Quadrant I or IV
    if (x >= 0.0f) {
        // If x = 0, then atanf(y/x) = +pi/2 if y > 0
        //                atanf(y/x) = -pi/2 if y < 0
        theta = atanf(y / x); // in [-pi/2, +pi/2]

        if (theta < 0.0f) theta += 2.0f * std::numbers::pi; // in [0, 2*pi).
    }

    // Quadrant II or III
    else
        theta = atanf(y / x) + std::numbers::pi; // in [0, 2*pi).

    return theta;
}

//------------------------------------------------------------------------
void GeometryGenerator::createBox(float pWidth, float pHeight, float pDepth, MeshData& pMeshData) {
    //
    // Create the vertices.
    //

    Vertex v[24];

    float w2 = 0.5f * pWidth;
    float h2 = 0.5f * pHeight;
    float d2 = 0.5f * pDepth;

    // Fill in the front face vertex data.
    v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // Fill in the back face vertex data.
    v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Fill in the top face vertex data.
    v[8]  = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[9]  = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // Fill in the bottom face vertex data.
    v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Fill in the left face vertex data.
    v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
    v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
    v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

    // Fill in the right face vertex data.
    v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
    v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

    pMeshData.Vertices.assign(&v[0], &v[24]);

    //
    // Create the indices.
    //

    unsigned int i[36];

    // Fill in the front face index data
    i[0] = 0;
    i[1] = 1;
    i[2] = 2;
    i[3] = 0;
    i[4] = 2;
    i[5] = 3;

    // Fill in the back face index data
    i[6]  = 4;
    i[7]  = 5;
    i[8]  = 6;
    i[9]  = 4;
    i[10] = 6;
    i[11] = 7;

    // Fill in the top face index data
    i[12] = 8;
    i[13] = 9;
    i[14] = 10;
    i[15] = 8;
    i[16] = 10;
    i[17] = 11;

    // Fill in the bottom face index data
    i[18] = 12;
    i[19] = 13;
    i[20] = 14;
    i[21] = 12;
    i[22] = 14;
    i[23] = 15;

    // Fill in the left face index data
    i[24] = 16;
    i[25] = 17;
    i[26] = 18;
    i[27] = 16;
    i[28] = 18;
    i[29] = 19;

    // Fill in the right face index data
    i[30] = 20;
    i[31] = 21;
    i[32] = 22;
    i[33] = 20;
    i[34] = 22;
    i[35] = 23;

    pMeshData.Indices.assign(&i[0], &i[36]);
}

//------------------------------------------------------------------------
void GeometryGenerator::createGrid(float        pGridWidth,
                                   float        pGridDepth,
                                   unsigned int pNbVertexWidth,
                                   unsigned int pNbVertexDepth,
                                   MeshData&    pMeshData) {
    pMeshData.Indices.clear();
    pMeshData.Vertices.clear();

    const unsigned int vertexCount = pNbVertexWidth * pNbVertexDepth;
    const unsigned int faceCount   = (pNbVertexWidth - 1) * (pNbVertexDepth - 1) * 2;

    const float halfWidth = 0.5f * pGridWidth;
    const float halfDepth = 0.5f * pGridDepth;

    const float dx = pGridWidth / (pNbVertexWidth - 1);
    const float dz = pGridDepth / (pNbVertexDepth - 1);

    const float du = 1.0f / (pNbVertexWidth - 1);
    const float dv = 1.0f / (pNbVertexDepth - 1);

    //
    // Create Vertices
    //
    pMeshData.Vertices.resize(vertexCount);

    for (unsigned int i = 0; i < pNbVertexDepth; i++) {
        float z = halfDepth - i * dz;
        for (unsigned int j = 0; j < pNbVertexWidth; j++) {
            float x = -halfWidth + j * dx;

            unsigned int index                 = i * pNbVertexWidth + j;
            pMeshData.Vertices[index].Position = glm::vec3(x, 0, z);

            pMeshData.Vertices[index].Normal   = glm::vec3(0.f, 1.f, 0.f);
            pMeshData.Vertices[index].TangentU = glm::vec3(1.f, 0.f, 0.f);

            pMeshData.Vertices[index].TexC.u = j * du;
            pMeshData.Vertices[index].TexC.v = i * dv;
        }
    }

    //
    // Create Indices
    //
    pMeshData.Indices.resize(faceCount * 3);
    unsigned int k = 0;
    for (unsigned int i = 0; i < pNbVertexDepth - 1; i++) {
        for (unsigned int j = 0; j < pNbVertexWidth - 1; j++) {
            pMeshData.Indices[k]     = i * pNbVertexWidth + j;
            pMeshData.Indices[k + 1] = i * pNbVertexWidth + j + 1;
            pMeshData.Indices[k + 2] = (i + 1) * pNbVertexWidth + j;
            pMeshData.Indices[k + 3] = (i + 1) * pNbVertexWidth + j;
            pMeshData.Indices[k + 4] = i * pNbVertexWidth + j + 1;
            pMeshData.Indices[k + 5] = (i + 1) * pNbVertexWidth + j + 1;

            k += 6;
        }
    }
}

//------------------------------------------------------------------------
void GeometryGenerator::createGeoSphere(float        pRadius,
                                        unsigned int pSubdivisionCount,
                                        MeshData&    pMeshData) {
    // Put a cap on the number of subdivisions.
    pSubdivisionCount = (unsigned int)std::min((float)pSubdivisionCount, (float)5u);

    // Approximate a sphere by tessellating an icosahedron.

    const float X = 0.525731f;
    const float Z = 0.850651f;

    glm::vec3 pos[12] = {
        glm::vec3(-X, 0.0f, Z), glm::vec3(X, 0.0f, Z),   glm::vec3(-X, 0.0f, -Z),
        glm::vec3(X, 0.0f, -Z), glm::vec3(0.0f, Z, X),   glm::vec3(0.0f, Z, -X),
        glm::vec3(0.0f, -Z, X), glm::vec3(0.0f, -Z, -X), glm::vec3(Z, X, 0.0f),
        glm::vec3(-Z, X, 0.0f), glm::vec3(Z, -X, 0.0f),  glm::vec3(-Z, -X, 0.0f)};

    unsigned long k[60] = {1, 4,  0, 4, 9, 0,  4, 5, 9,  8, 5, 4,  1,  8,  4, 1, 10, 8,  10, 3,
                           8, 8,  3, 5, 3, 2,  5, 3, 7,  2, 3, 10, 7,  10, 6, 7, 6,  11, 7,  6,
                           0, 11, 6, 1, 0, 10, 1, 6, 11, 0, 9, 2,  11, 9,  5, 2, 9,  11, 2,  7};

    pMeshData.Vertices.resize(12);
    pMeshData.Indices.resize(60);

    for (unsigned int i = 0; i < 12; ++i) pMeshData.Vertices[i].Position = pos[i];

    for (unsigned int i = 0; i < 60; ++i) pMeshData.Indices[i] = k[i];

    for (unsigned int i = 0; i < pSubdivisionCount; ++i) _subdivide(pMeshData);

    // Project vertices onto sphere and scale.
    for (unsigned int i = 0; i < pMeshData.Vertices.size(); ++i) {
        // Project onto unit sphere.
        glm::vec3 n = glm::normalize(glm::vec3(pMeshData.Vertices[i].Position));
        // XMVECTOR n = XMVector3Normalize( XMLoadFloat3( &pMeshData.Vertices[i].Position) );

        // Project onto sphere.
        glm::vec3 p = pRadius * n;

        // XMStoreFloat3(&pMeshData.Vertices[i].Position, p);
        // XMStoreFloat3(&pMeshData.Vertices[i].Normal, n);

        pMeshData.Vertices[i].Position = p;
        pMeshData.Vertices[i].Normal   = n;

        // Derive texture coordinates from spherical coordinates.
        float theta =
            AngleFromXY(pMeshData.Vertices[i].Position.x, pMeshData.Vertices[i].Position.z);

        float phi = acosf(pMeshData.Vertices[i].Position.y / pRadius);

        pMeshData.Vertices[i].TexC.u = theta / std::numbers::pi;
        pMeshData.Vertices[i].TexC.v = phi / std::numbers::pi;

        // Partial derivative of P with respect to theta
        pMeshData.Vertices[i].TangentU.x = -pRadius * sinf(phi) * sinf(theta);
        pMeshData.Vertices[i].TangentU.y = 0.0f;
        pMeshData.Vertices[i].TangentU.z = +pRadius * sinf(phi) * cosf(theta);

        // XMVECTOR T = XMLoadFloat3(&meshData.Vertices[i].TangentU);
        // XMStoreFloat3(&meshData.Vertices[i].TangentU, XMVector3Normalize(T));
        pMeshData.Vertices[i].TangentU = glm::normalize(pMeshData.Vertices[i].TangentU);
    }
}

//------------------------------------------------------------------------
void GeometryGenerator::createCylinder(float        pBottomRadius,
                                       float        pTopRadius,
                                       float        pHeight,
                                       unsigned int pSliceCount,
                                       unsigned int pStackCount,
                                       MeshData&    pMeshData) {
    pMeshData.Indices.clear();
    pMeshData.Vertices.clear();

    const float lStackHeight = pHeight / pStackCount;
    const float lRadiusStep  = (pTopRadius - pBottomRadius) / pStackCount;

    const unsigned int lRingCount = pStackCount + 1;

    for (unsigned int i = 0; i < lRingCount; i++) {
        float y = -0.5f * pHeight + i * lStackHeight;
        float r = pBottomRadius + i * lRadiusStep;

        // Vertice of ring
        float dTheta = 2.0f * std::numbers::pi / pSliceCount;
        for (unsigned int j = 0; j <= pSliceCount; j++) {
            Vertex lVertex;

            float c = std::cos(j * dTheta);
            float s = std::sin(j * dTheta);

            lVertex.Position = glm::vec3(r * c, y, r * s);
            lVertex.TexC.u   = (float)j / pSliceCount;
            lVertex.TexC.v   = 1.0f - (float)i / pStackCount;

            // This is unit length.
            lVertex.TangentU = glm::vec3(-s, 0.0f, c);

            float          dr = pBottomRadius - pTopRadius;
            glm::vec3 bitangent(dr * c, -pHeight, dr * s);

            glm::vec3 T = lVertex.TangentU;
            glm::vec3 B = bitangent;
            glm::vec3 N = glm::normalize(glm::cross(T,B)); //T.crossProduct(B).normalized();
            lVertex.Normal   = N;

            pMeshData.Vertices.push_back(lVertex);
        }
    }

    //
    // Cr�ation des index
    //
    const unsigned int lRingVertexCount = pSliceCount + 1;
    for (unsigned int i = 0; i < pStackCount; i++) {
        for (unsigned int j = 0; j < pSliceCount; j++) {
            pMeshData.Indices.push_back(i * lRingVertexCount + j);
            pMeshData.Indices.push_back((i + 1) * lRingVertexCount + j);
            pMeshData.Indices.push_back((i + 1) * lRingVertexCount + j + 1);

            pMeshData.Indices.push_back(i * lRingVertexCount + j);
            pMeshData.Indices.push_back((i + 1) * lRingVertexCount + j + 1);
            pMeshData.Indices.push_back(i * lRingVertexCount + j + 1);
        }
    }

    _buildCylinderTopCap(pTopRadius, pHeight, pSliceCount, pMeshData);
    _buildCylinderBottomCap(pBottomRadius, pHeight, pStackCount, pMeshData);
}

//------------------------------------------------------------------------
void GeometryGenerator::createSphere(float        pRadius,
                                     unsigned int pSliceCount,
                                     unsigned int pStackCount,
                                     MeshData&    pMeshData) {
    pMeshData.Vertices.clear();
    pMeshData.Indices.clear();

    //
    // Compute the vertices stating at the top pole and moving down the stacks.
    //

    // Poles: note that there will be texture coordinate distortion as there is
    // not a unique point on the texture map to assign to the pole when mapping
    // a rectangular texture onto a sphere.
    Vertex topVertex(0.0f, +pRadius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Vertex bottomVertex(0.0f, -pRadius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    pMeshData.Vertices.push_back(topVertex);

    float phiStep   = std::numbers::pi / pStackCount;
    float thetaStep = 2.0f * std::numbers::pi / pSliceCount;

    // Compute vertices for each stack ring (do not count the poles as rings).
    for (unsigned int i = 1; i <= pStackCount - 1; ++i) {
        float phi = i * phiStep;

        // Vertices of ring.
        for (unsigned int j = 0; j <= pSliceCount; ++j) {
            float theta = j * thetaStep;

            Vertex v;

            // spherical to cartesian
            v.Position.x = pRadius * sinf(phi) * cosf(theta);
            v.Position.y = pRadius * cosf(phi);
            v.Position.z = pRadius * sinf(phi) * sinf(theta);

            // Partial derivative of P with respect to theta
            v.TangentU.x = -pRadius * sinf(phi) * sinf(theta);
            v.TangentU.y = 0.0f;
            v.TangentU.z = +pRadius * sinf(phi) * cosf(theta);

            glm::normalize(v.TangentU);

            v.Normal = glm::normalize(v.Position);

            v.TexC.u = theta / std::numbers::pi;
            v.TexC.v = phi / std::numbers::pi;

            pMeshData.Vertices.push_back(v);
        }
    }

    pMeshData.Vertices.push_back(bottomVertex);

    //
    // Compute indices for top stack.  The top stack was written first to the vertex buffer
    // and connects the top pole to the first ring.
    //

    for (unsigned int i = 1; i <= pSliceCount; ++i) {
        pMeshData.Indices.push_back(0);
        pMeshData.Indices.push_back(i + 1);
        pMeshData.Indices.push_back(i);
    }

    //
    // Compute indices for inner stacks (not connected to poles).
    //

    // Offset the indices to the index of the first vertex in the first ring.
    // This is just skipping the top pole vertex.
    unsigned int baseIndex       = 1;
    unsigned int ringVertexCount = pSliceCount + 1;
    for (unsigned int i = 0; i < pSliceCount - 2; ++i) {
        for (unsigned int j = 0; j < pSliceCount; ++j) {
            pMeshData.Indices.push_back(baseIndex + i * ringVertexCount + j);
            pMeshData.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            pMeshData.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            pMeshData.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            pMeshData.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            pMeshData.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }

    //
    // Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
    // and connects the bottom pole to the bottom ring.
    //

    // South pole vertex was added last.
    unsigned int southPoleIndex = (unsigned int)pMeshData.Vertices.size() - 1;

    // Offset the indices to the index of the first vertex in the last ring.
    baseIndex = southPoleIndex - ringVertexCount;

    for (unsigned int i = 0; i < pSliceCount; ++i) {
        pMeshData.Indices.push_back(southPoleIndex);
        pMeshData.Indices.push_back(baseIndex + i);
        pMeshData.Indices.push_back(baseIndex + i + 1);
    }
}

//------------------------------------------------------------------------
void GeometryGenerator::createFullscreenQuad(MeshData& meshData) {
    meshData.Vertices.resize(4);
    meshData.Indices.resize(6);

    // Position coordinates specified in NDC space.
    meshData.Vertices[0] =
        Vertex(-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    meshData.Vertices[1] =
        Vertex(-1.0f, +1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    meshData.Vertices[2] =
        Vertex(+1.0f, +1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    meshData.Vertices[3] =
        Vertex(+1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    meshData.Indices[0] = 0;
    meshData.Indices[1] = 1;
    meshData.Indices[2] = 2;

    meshData.Indices[3] = 0;
    meshData.Indices[4] = 2;
    meshData.Indices[5] = 3;
}

//------------------------------------------------------------------------
void GeometryGenerator::_subdivide(MeshData& meshData) {
    // Save a copy of the input geometry.
    MeshData inputCopy = meshData;

    meshData.Vertices.resize(0);
    meshData.Indices.resize(0);

    //       v1
    //       *
    //      / \
	//     /   \
	//  m0*-----*m1
    //   / \   / \
	//  /   \ /   \
	// *-----*-----*
    // v0    m2     v2

    unsigned int numTris = (unsigned int)inputCopy.Indices.size() / 3;
    for (unsigned int i = 0; i < numTris; ++i) {
        Vertex v0 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 0]];
        Vertex v1 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 1]];
        Vertex v2 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 2]];

        //
        // Generate the midpoints.
        //

        Vertex m0, m1, m2;

        // For subdivision, we just care about the position component.  We derive the other
        // vertex components in CreateGeosphere.

        m0.Position = glm::vec3(0.5f * (v0.Position.x + v1.Position.x),
                                     0.5f * (v0.Position.y + v1.Position.y),
                                     0.5f * (v0.Position.z + v1.Position.z));

        m1.Position = glm::vec3(0.5f * (v1.Position.x + v2.Position.x),
                                     0.5f * (v1.Position.y + v2.Position.y),
                                     0.5f * (v1.Position.z + v2.Position.z));

        m2.Position = glm::vec3(0.5f * (v0.Position.x + v2.Position.x),
                                     0.5f * (v0.Position.y + v2.Position.y),
                                     0.5f * (v0.Position.z + v2.Position.z));

        //
        // Add new geometry.
        //

        meshData.Vertices.push_back(v0); // 0
        meshData.Vertices.push_back(v1); // 1
        meshData.Vertices.push_back(v2); // 2
        meshData.Vertices.push_back(m0); // 3
        meshData.Vertices.push_back(m1); // 4
        meshData.Vertices.push_back(m2); // 5

        meshData.Indices.push_back(i * 6 + 0);
        meshData.Indices.push_back(i * 6 + 3);
        meshData.Indices.push_back(i * 6 + 5);

        meshData.Indices.push_back(i * 6 + 3);
        meshData.Indices.push_back(i * 6 + 4);
        meshData.Indices.push_back(i * 6 + 5);

        meshData.Indices.push_back(i * 6 + 5);
        meshData.Indices.push_back(i * 6 + 4);
        meshData.Indices.push_back(i * 6 + 2);

        meshData.Indices.push_back(i * 6 + 3);
        meshData.Indices.push_back(i * 6 + 1);
        meshData.Indices.push_back(i * 6 + 4);
    }
}

//------------------------------------------------------------------------
void GeometryGenerator::_buildCylinderTopCap(float        topRadius,
                                             float        height,
                                             unsigned int sliceCount,
                                             MeshData&    meshData) {
    unsigned int baseIndex = (unsigned int)meshData.Vertices.size();

    float y      = 0.5f * height;
    float dTheta = 2.0f * std::numbers::pi / sliceCount;

    // Duplicate cap ring vertices because the texture coordinates and normals differ.
    for (unsigned int i = 0; i <= sliceCount; ++i) {
        float x = topRadius * cosf(i * dTheta);
        float z = topRadius * sinf(i * dTheta);

        // Scale down by the height to try and make top cap texture coord area
        // proportional to base.
        float u = x / height + 0.5f;
        float v = z / height + 0.5f;

        meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
    }

    // Cap center vertex.
    meshData.Vertices.push_back(
        Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

    // Index of center vertex.
    unsigned int centerIndex = (unsigned int)meshData.Vertices.size() - 1;

    for (unsigned int i = 0; i < sliceCount; ++i) {
        meshData.Indices.push_back(centerIndex);
        meshData.Indices.push_back(baseIndex + i + 1);
        meshData.Indices.push_back(baseIndex + i);
    }
}

//------------------------------------------------------------------------
void GeometryGenerator::_buildCylinderBottomCap(float        bottomRadius,
                                                float        height,
                                                unsigned int sliceCount,
                                                MeshData&    meshData) {
    //
    // Build bottom cap.
    //

    unsigned int baseIndex = (unsigned int)meshData.Vertices.size();
    float        y         = -0.5f * height;

    // vertices of ring
    float dTheta = 2.0f * std::numbers::pi / sliceCount;
    for (unsigned int i = 0; i <= sliceCount; ++i) {
        float x = bottomRadius * cosf(i * dTheta);
        float z = bottomRadius * sinf(i * dTheta);

        // Scale down by the height to try and make top cap texture coord area
        // proportional to base.
        float u = x / height + 0.5f;
        float v = z / height + 0.5f;

        meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
    }

    // Cap center vertex.
    meshData.Vertices.push_back(
        Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

    // Cache the index of center vertex.
    unsigned int centerIndex = (unsigned int)meshData.Vertices.size() - 1;

    for (unsigned int i = 0; i < sliceCount; ++i) {
        meshData.Indices.push_back(centerIndex);
        meshData.Indices.push_back(baseIndex + i);
        meshData.Indices.push_back(baseIndex + i + 1);
    }
}
