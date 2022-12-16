#ifndef CXUTIL_VERTEX_1607076481387_H
#define CXUTIL_VERTEX_1607076481387_H
#include "cxutil/math/Point3.h"
#include "cxutil/math/AABB3D.h"

namespace cxutil
{
    class MeshVertex
    {
    public:
        Point3 p;
        std::vector<uint32_t> connected_faces;

        MeshVertex(Point3 p) : p(p) { connected_faces.reserve(8); }
    };

    class MeshFace
    {
    public:
        int vertex_index[3] = { -1 };
        int connected_face_index[3];
    };

    class SlicerSegment
    {
    public:
        Point start, end;
        int faceIndex = -1;
        // The index of the other face connected via the edge that created end
        int endOtherFaceIdx = -1;
        // If end corresponds to a vertex of the mesh, then this is populated
        // with the vertex that it ended on.
        const MeshVertex* endVertex = nullptr;
        bool addedToPolygon = false;
    };
}

#endif // CXUTIL_VERTEX_1607076481387_H