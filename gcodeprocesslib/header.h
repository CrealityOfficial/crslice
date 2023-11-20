#ifndef GCODEPROCESSHEADER_1632383314974_H
#define GCODEPROCESSHEADER_1632383314974_H

#include "ccglobal/tracer.h"
#include "trimesh2/TriMesh.h"

namespace gcode
{
    class  SliceLine3D
    {
    public:
        SliceLine3D() {};
        ~SliceLine3D() {};

        trimesh::vec3 start;
        trimesh::vec3 end;
    };

    enum class SliceCompany
    {
        none,creality, cura, prusa, bambu, ideamaker, superslicer, ffslicer, simplify
    };

}


#endif // GCODEPROCESSHEADER_1632383314974_H