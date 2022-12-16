//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef INFILL_UNIFORM_DENSITY_PROVIDER_H_520
#define INFILL_UNIFORM_DENSITY_PROVIDER_H_520

#include "DensityProvider.h"

namespace cura52
{

struct AABB3D;

class UniformDensityProvider : public DensityProvider
{
public:
    UniformDensityProvider(float density)
    : density(density)
    {
    };

    virtual ~UniformDensityProvider()
    {
    };

    virtual float operator()(const AABB3D&) const
    {
        return density;
    };
protected:
    float density;
};

} // namespace cura52


#endif // INFILL_UNIFORM_DENSITY_PROVIDER_H
