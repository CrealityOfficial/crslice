//Copyright (C) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#include <string.h>
#include <stdio.h>
#include <limits>

#include "MeshGroup.h"
#include "mesh.h"
#include "cxutil/math/floatpoint.h"
#include "cxutil/settings/Settings.h"

namespace cxutil
{
    FILE* binaryMeshBlob = nullptr;

    MeshGroup::MeshGroup()
        :settings(new Settings())
    {

    }

    MeshGroup::MeshGroup(const MeshGroup& group)
        : settings(new Settings())
    {

    }

    MeshGroup::~MeshGroup()
    {
        clear();

        for (Mesh* m : meshes)
        {
            delete m;
        }

        delete settings;
    }

    Point3 MeshGroup::min() const
    {
        if (meshes.size() < 1)
        {
            return Point3(0, 0, 0);
        }
        Point3 ret(std::numeric_limits<coord_t>::max(), std::numeric_limits<coord_t>::max(), std::numeric_limits<coord_t>::max());
        for (const Mesh* mesh : meshes)
        {
            if (mesh->settings->get<bool>("infill_mesh") || mesh->settings->get<bool>("cutting_mesh") || mesh->settings->get<bool>("anti_overhang_mesh")) //Don't count pieces that are not printed.
            {
                continue;
            }
            Point3 v = mesh->min();
            ret.x = std::min(ret.x, v.x);
            ret.y = std::min(ret.y, v.y);
            ret.z = std::min(ret.z, v.z);
        }
        return ret;
    }

    Point3 MeshGroup::max() const
    {
        if (meshes.size() < 1)
        {
            return Point3(0, 0, 0);
        }
        Point3 ret(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min());
        for (const Mesh* mesh : meshes)
        {
            if (mesh->settings->get<bool>("infill_mesh") || mesh->settings->get<bool>("cutting_mesh") || mesh->settings->get<bool>("anti_overhang_mesh")) //Don't count pieces that are not printed.
            {
                continue;
            }
            Point3 v = mesh->max();
            ret.x = std::max(ret.x, v.x);
            ret.y = std::max(ret.y, v.y);
            ret.z = std::max(ret.z, v.z);
        }
        return ret;
    }

    void MeshGroup::clear()
    {
        for (Mesh* m : meshes)
        {
            m->clear();
        }
    }

    void MeshGroup::finalize()
    {
        //If the machine settings have been supplied, offset the given position vertices to the center of vertices (0,0,0) is at the bed center.
        Point3 meshgroup_offset(0, 0, 0);
        if (!settings->get<bool>("machine_center_is_zero"))
        {
            meshgroup_offset.x = settings->get<coord_t>("machine_width") / 2;
            meshgroup_offset.y = settings->get<coord_t>("machine_depth") / 2;
        }
        // If a mesh position was given, put the mesh at this position in 3D space. 
        for (Mesh* mesh : meshes)
        {
            Point3 mesh_offset(mesh->settings->get<coord_t>("mesh_position_x"), mesh->settings->get<coord_t>("mesh_position_y"), mesh->settings->get<coord_t>("mesh_position_z"));
            if (mesh->settings->get<bool>("center_object"))
            {
                Point3 object_min = mesh->min();
                Point3 object_max = mesh->max();
                Point3 object_size = object_max - object_min;
                mesh_offset += Point3(-object_min.x - object_size.x / 2, -object_min.y - object_size.y / 2, -object_min.z);
            }
            mesh->offset(mesh_offset + meshgroup_offset);
        }
    }
}//namespace cxutil
