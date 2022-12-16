#include "polygontrimmer.h"
#include "cxutil/input/param.h"
#include "cxutil/input/meshinput.h"

namespace cxutil
{
	void overhangPrintable(SlicedMesh& slicedMesh, MeshParam* param)
	{
        const AngleRadians angle = param->conical_overhang_angle;
        const double tan_angle = tan(angle);  // the XY-component of the angle
        const coord_t layer_thickness = param->layer_height;
        coord_t max_dist_from_lower_layer = tan_angle * layer_thickness; // max dist which can be bridged

        int layerCount = (int)slicedMesh.m_layers.size();
        for (int layer_nr = layerCount - 2; layer_nr >= 0; layer_nr--)
        {
            SlicedMeshLayer& layer = slicedMesh.m_layers[layer_nr];
            SlicedMeshLayer& layer_above = slicedMesh.m_layers[layer_nr + 1];
            if (std::abs(max_dist_from_lower_layer) < 5)
            { // magically nothing happens when max_dist_from_lower_layer == 0
                // below magic code solves that
                int safe_dist = 20;
                Polygons diff = layer_above.polygons.difference(layer.polygons.offset(-safe_dist));
                layer.polygons = layer.polygons.unionPolygons(diff);
                layer.polygons = layer.polygons.smooth(safe_dist);
                layer.polygons.simplify(safe_dist, safe_dist * safe_dist / 4);
                // somehow layer.polygons get really jagged lines with a lot of vertices
                // without the above steps slicing goes really slow
            }
            else
            {
                layer.polygons = layer.polygons.unionPolygons(layer_above.polygons.offset(-max_dist_from_lower_layer));
            }
        }
	}

    void carveCuttingMeshes(std::vector<SlicedMesh>& slicedMesh, std::vector<MeshInput*>& meshInputs)
    {
        size_t meshCount = slicedMesh.size();
        std::vector<MeshParam*> meshParams;
        for (size_t i = 0; i < meshCount; ++i)
            meshParams.push_back(meshInputs.at(i)->param());
        for (unsigned int carving_mesh_idx = 0; carving_mesh_idx < meshCount; carving_mesh_idx++)
        {
            MeshParam* cutting_mesh = meshParams[carving_mesh_idx];
            if (!cutting_mesh->isCuttingMesh())
            {
                continue;
            }

            SlicedMesh& cutting_mesh_volume = slicedMesh[carving_mesh_idx];
            size_t layerCount = cutting_mesh_volume.m_layers.size();
            for (unsigned int layer_nr = 0; layer_nr < layerCount; layer_nr++)
            {
                Polygons& cutting_mesh_layer = cutting_mesh_volume.m_layers[layer_nr].polygons;
                Polygons new_outlines;
                for (unsigned int carved_mesh_idx = 0; carved_mesh_idx < meshCount; carved_mesh_idx++)
                {
                    MeshParam* carved_mesh = meshParams[carved_mesh_idx];
                    //Do not apply cutting_mesh for meshes which have settings (cutting_mesh, anti_overhang_mesh, support_mesh).
                    if (carved_mesh->isCuttingMesh() || carved_mesh->isAntiOverhang()|| carved_mesh->isSupportMesh())
                    {
                        continue;
                    }
                    SlicedMesh& carved_volume = slicedMesh[carved_mesh_idx];
                    Polygons& carved_mesh_layer = carved_volume.m_layers[layer_nr].polygons;
                    Polygons intersection = cutting_mesh_layer.intersection(carved_mesh_layer);
                    new_outlines.add(intersection);
                    carved_mesh_layer = carved_mesh_layer.difference(cutting_mesh_layer);
                }
                cutting_mesh_layer = new_outlines.unionPolygons();
            }
        }
    }

    void carveMultipleVolumes(std::vector<SlicedMesh>& slicedMesh, std::vector<MeshInput*>& meshInputs, bool alternate_carve_order)
    {
        size_t meshCount = slicedMesh.size();
        std::vector<MeshParam*> meshParams;
        std::vector<SlicedMesh*> ranked_volumes;
        for (size_t i = 0; i < meshCount; ++i)
        {
            meshParams.push_back(meshInputs.at(i)->param());
            ranked_volumes.push_back(&slicedMesh.at(i));
        }
        //Go trough all the volumes, and remove the previous volume outlines from our own outline, so we never have overlapped areas.
        //std::sort(ranked_volumes.begin(), ranked_volumes.end(),
        //    [](Slicer* volume_1, Slicer* volume_2)
        //    {
        //        return volume_1->mesh->settings.get<int>("infill_mesh_order") < volume_2->mesh->settings.get<int>("infill_mesh_order");
        //    });
        for (unsigned int volume_1_idx = 1; volume_1_idx < meshCount; volume_1_idx++)
        {
            SlicedMesh& volume_1 = *ranked_volumes[volume_1_idx];
            MeshParam* param1 = meshParams.at(volume_1_idx);
            if (param1->isInfill()
                || param1->isAntiOverhang()
                || param1->isSupportMesh()
                || param1->magic_mesh_surface_mode == ESurfaceMode::SURFACE
                )
            {
                continue;
            }
            for (unsigned int volume_2_idx = 0; volume_2_idx < volume_1_idx; volume_2_idx++)
            {
                SlicedMesh& volume_2 = *ranked_volumes[volume_2_idx];
                MeshParam* param2 = meshParams.at(volume_2_idx);
                if (param2->isInfill()
                    || param2->isAntiOverhang()
                    || param2->isSupportMesh()
                    || param2->magic_mesh_surface_mode == ESurfaceMode::SURFACE
                    )
                {
                    continue;
                }
                if (!meshInputs.at(volume_1_idx)->box().hit(meshInputs.at(volume_2_idx)->box()))
                {
                    continue;
                }
                for (unsigned int layerNr = 0; layerNr < volume_1.m_layers.size(); layerNr++)
                {
                    SlicedMeshLayer& layer1 = volume_1.m_layers[layerNr];
                    SlicedMeshLayer& layer2 = volume_2.m_layers[layerNr];
                    if (alternate_carve_order && layerNr % 2 == 0 
                        && param1->infill_mesh_order == param2->infill_mesh_order)
                    {
                        layer2.polygons = layer2.polygons.difference(layer1.polygons);
                    }
                    else
                    {
                        layer1.polygons = layer1.polygons.difference(layer2.polygons);
                    }
                }
            }
        }
    }

    void generateMultipleVolumesOverlap(std::vector<SlicedMesh>& slicedMesh, std::vector<MeshInput*>& meshInputs)
    {
        int offset_to_merge_other_merged_volumes = 20;
        size_t meshCount = slicedMesh.size();
        std::vector<MeshParam*> meshParams;
        for (size_t i = 0; i < meshCount; ++i)
            meshParams.push_back(meshInputs.at(i)->param());

        for (size_t i = 0; i < meshCount; ++i)
        {
            SlicedMesh* volume = &slicedMesh.at(i);
            MeshParam* param = meshParams.at(i);
            coord_t overlap = param->multiple_mesh_overlap;
            if (param->isInfill()
                || param->isAntiOverhang()
                || param->isSupportMesh()
                || overlap == 0)
            {
                continue;
            }
            AABB3D aabb(meshInputs.at(i)->box());
            aabb.expandXY(overlap); // expand to account for the case where two models and their bounding boxes are adjacent along the X or Y-direction
            size_t layerSize = volume->m_layers.size();
            for (unsigned int layer_nr = 0; layer_nr < layerSize; layer_nr++)
            {
                Polygons all_other_volumes;
                for (size_t j = 0; j < meshCount; ++j)
                {
                    SlicedMesh* other_volume = &slicedMesh.at(j);
                    MeshParam* otherParam = meshParams.at(j);
                    if (otherParam->isInfill()
                        || otherParam->isAntiOverhang()
                        || otherParam->isSupportMesh()
                        || !meshInputs.at(j)->box().hit(aabb)
                        || other_volume == volume
                        )
                    {
                        continue;
                    }
                    SlicedMeshLayer& other_volume_layer = other_volume->m_layers[layer_nr];
                    all_other_volumes = all_other_volumes.unionPolygons(other_volume_layer.polygons.offset(offset_to_merge_other_merged_volumes));
                }

                SlicedMeshLayer& volume_layer = volume->m_layers[layer_nr];
                volume_layer.polygons = volume_layer.polygons.unionPolygons(all_other_volumes.intersection(volume_layer.polygons.offset(overlap / 2)));
            }
        }
    }
}