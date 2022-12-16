#ifndef CURA_LOADMESH_1605144235165_H
#define CURA_LOADMESH_1605144235165_H
#include "cxutil/math/floatpoint.h"
#include "cxutil/slicer/ExtruderTrain.h"

namespace cxutil
{
    class MeshGroup;
    class Mesh;
/*!
 * Load a Mesh from file and store it in the \p meshgroup.
 *
 * \param meshgroup The meshgroup where to store the mesh
 * \param filename The filename of the mesh file
 * \param transformation The transformation applied to all vertices
 * \param object_parent_settings (optional) The parent settings object of the new mesh. Defaults to \p meshgroup if none is given.
 * \return whether the file could be loaded
 */
    bool loadMeshIntoMeshGroup(MeshGroup* meshgroup, const char* filename,
        const FMatrix4x4& transformation, Settings* object_parent_settings);

    Mesh* loadMeshSTL(const char* filename, const FMatrix4x4& matrix, Settings* object_parent_settings);
}

#endif // CURA_LOADMESH_1605144235165_H