//  Copyright (c) 2018-2022 Ultimaker B.V.
//  CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef ENDDDDD_DEBUGGER_H
#define ENDDDDD_DEBUGGER_H
#include "utils/polygon.h"
#include "sliceDataStorage.h"

namespace cura52
{
    class Debugger
    {
    public:
        virtual ~Debugger() {}

        virtual void startSlice(int count) = 0;
        virtual void startGroup(int index) = 0;
        virtual void groupBox(const Point3& _min, const Point3& _max) = 0;

        virtual void sliceLayerCount(int count) = 0;
        virtual void sliceLayerData(int index, int z, const Polygons& polygons, const Polygons& openPolygons) = 0;

        virtual void parts(const SliceMeshStorage& storage) = 0;
    };

} //namespace cura52

#endif //ENDDDDD_DEBUGGER_H

