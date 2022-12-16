//Copyright (C) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef CXUTIL_MESH_GROUP_H
#define CXUTIL_MESH_GROUP_H
#include "cxutil/util/NoCopy.h"
#include "cxutil/math/IntPoint.h"

namespace cxutil
{
    class Mesh;
    class Settings;
    /*!
     * A MeshGroup is a collection with 1 or more 3D meshes.
     *
     * One MeshGroup is a whole which is printed at once.
     * Generally there is one single MeshGroup, though when using one-at-a-time printing, multiple MeshGroups are processed consecutively.
     */
    class MeshGroup
    {
    public:
        std::vector<Mesh*> meshes;
        Point3 m_offset;
        Settings* settings;

        Point3 min() const; //! minimal corner of bounding box
        Point3 max() const; //! maximal corner of bounding box

        MeshGroup();
        MeshGroup(const MeshGroup& group);
        ~MeshGroup();

        void clear();

        void finalize();
    };
} //namespace cxutil

#endif //CXUTIL_MESH_GROUP_H
