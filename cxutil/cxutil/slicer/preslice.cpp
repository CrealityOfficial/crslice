#include "preslice.h"
#include "cxutil/input/groupinput.h"
#include "cxutil/input/dlpinput.h"
#include "trimesh2/TriMesh.h"
#include <float.h>
#include "cxutil/settings/AdaptiveLayerHeights.h"
namespace cxutil
{
	void buildSliceInfos(GroupInput* meshGroup, std::vector<int>& z, std::vector<coord_t>& printZ, std::vector<coord_t>& thicknesses)
	{
        MeshGroupParam* groupParam = meshGroup->param();
        AABB3D box = meshGroup->box();

        int slice_layer_count = 0;
        const coord_t initial_layer_thickness = groupParam->layer_height_0;
        const coord_t layer_thickness = groupParam->layer_height;
        const bool use_variable_layer_heights = groupParam->adaptive_layer_height_enabled;

        AdaptiveLayerHeights* adaptive_layer_heights = nullptr;
        if (use_variable_layer_heights)
        {
            // Calculate adaptive layer heights
            const coord_t variable_layer_height_max_variation = groupParam->adaptive_layer_height_variation;
            const coord_t variable_layer_height_variation_step = groupParam->adaptive_layer_height_variation_step;
            const coord_t adaptive_threshold = groupParam->adaptive_layer_height_threshold;
            adaptive_layer_heights = new AdaptiveLayerHeights(layer_thickness, variable_layer_height_max_variation,
                variable_layer_height_variation_step, adaptive_threshold, nullptr);
            
            // Get the amount of layers
            slice_layer_count = adaptive_layer_heights->getLayerCount();
        }
        else
        {
            slice_layer_count = (box.max.z - initial_layer_thickness) / layer_thickness + 1;
        }

        if (slice_layer_count > 0)
        {
            z.resize(slice_layer_count, 0);
            printZ.resize(slice_layer_count, 0);
            thicknesses.resize(slice_layer_count, 0);

            // set (and initialize compensation for) initial layer, depending on slicing mode
            std::vector<AdaptiveLayer>* adaptiveLayers = adaptive_layer_heights ? 
                adaptive_layer_heights->getLayers() : nullptr;
            z[0] = std::max(0LL, initial_layer_thickness - layer_thickness);
            coord_t adjusted_layer_offset = initial_layer_thickness;
            if (use_variable_layer_heights)
            {
                z[0] = adaptiveLayers->at(0).z_position;
            }
            else
            {
                z[0] = initial_layer_thickness / 2;
                adjusted_layer_offset = initial_layer_thickness + (layer_thickness / 2);
            }

            // define all layer z positions (depending on slicing mode, see above)
            for (int layer_nr = 1; layer_nr < slice_layer_count; layer_nr++)
            {
                if (use_variable_layer_heights)
                {
                    z[layer_nr] = adaptiveLayers->at(layer_nr).z_position;
                }
                else
                {
                    z[layer_nr] = adjusted_layer_offset + (layer_thickness * (layer_nr - 1));
                }
            }

            for (int layer_nr = 0; layer_nr < slice_layer_count; layer_nr++)
            {
                if (use_variable_layer_heights)
                {
                    printZ[layer_nr] = adaptiveLayers->at(layer_nr).z_position;
                    thicknesses[layer_nr] = adaptiveLayers->at(layer_nr).layer_height;
                }
                else
                {
                    printZ[layer_nr] = initial_layer_thickness + (layer_nr * layer_thickness);

                    if (layer_nr == 0)
                    {
                        thicknesses[layer_nr] = initial_layer_thickness;
                    }
                    else
                    {
                        thicknesses[layer_nr] = layer_thickness;
                    }
                }

                // add the raft offset to each layer
                //if (has_raft)
                //{
                //    const ExtruderTrain& train = mesh_group_settings.get<ExtruderTrain&>("adhesion_extruder_nr");
                //    layer.printZ +=
                //        Raft::getTotalThickness()
                //        + train.settings.get<coord_t>("raft_airgap")
                //        - train.settings.get<coord_t>("layer_0_z_overlap"); // shift all layers (except 0) down
                //
                //    if (layer_nr == 0)
                //    {
                //        layer.printZ += train.settings.get<coord_t>("layer_0_z_overlap"); // undo shifting down of first layer
                //    }
                //}
            }
        }

        if (adaptive_layer_heights)
        {
            delete adaptive_layer_heights;
        }
	}

    coord_t getMaxZ(DLPInput* input)
    {
        float maxZ = DBL_MIN, minZ = DBL_MAX;
        std::vector<trimesh::TriMesh*> trimeshesSrc = input->getMeshesSrc();
        for (trimesh::TriMesh* meshesSrc : trimeshesSrc)
        {
            maxZ = std::max(meshesSrc->bbox.max.z, maxZ);
            minZ = std::min(meshesSrc->bbox.min.z, minZ);
        }
        coord_t Zdiff = MM2INT(maxZ - minZ);
        return Zdiff;
    }

    void buildSliceInfos(DLPInput* input, std::vector<int>& z)
    {
        if (!input) return;
        std::vector<MeshObjectPtr>& meshes = input->meshes();

        DLPParam& param = input->param();

        int slice_layer_count = 0;
        const coord_t initial_layer_thickness = param.initial_layer_thickness;
        const coord_t layer_thickness = param.layer_thickness;

#ifndef USE_TRIMESH_SLICE
        AABB3D box = input->box();
        slice_layer_count = (box.max.z - initial_layer_thickness) / layer_thickness + 1;
#else
        slice_layer_count = (getMaxZ(input) - initial_layer_thickness) / layer_thickness + 1;
#endif 

        if (slice_layer_count > 0)
        {
            z.resize(slice_layer_count, 0);

            z[0] = std::max(0LL, initial_layer_thickness - layer_thickness);
            coord_t adjusted_layer_offset = initial_layer_thickness;

            z[0] = initial_layer_thickness / 2;
            adjusted_layer_offset = initial_layer_thickness + (layer_thickness / 2);

            // define all layer z positions (depending on slicing mode, see above)
            for (int layer_nr = 1; layer_nr < slice_layer_count; layer_nr++)
            {
                z[layer_nr] = adjusted_layer_offset + (layer_thickness * (layer_nr - 1));
            }
        }
    }
}
