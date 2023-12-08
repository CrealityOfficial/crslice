#include "paintdata.h"
#include "communication/mesh.h"
#include "communication/sliceDataStorage.h"
#include "communication/slicecontext.h"
#include "communication/layerPart.h"
#include "settings/Settings.h"
#include "slice/sliceddata.h"
#include "slice/slicestep.h"
#include <fstream> 

namespace cura52
{
    void getBinaryData(std::string fileName, std::vector<Mesh>& meshs, Settings& setting)
    {
        std::fstream in(fileName, std::ios::in | std::ios::binary);
        if (in.is_open())
        {
            while (1)
            {
                int pNum = 0;
                in.read((char*)&pNum, sizeof(int));
                if (pNum > 0)
                {
                    meshs.push_back(Mesh(setting));
                    meshs.back().faces.resize(pNum);
                    for (int i = 0; i < pNum; ++i)
                    {
                        int num = 0;
                        in.read((char*)&num, sizeof(int));
                        if (num == 3)
                        {
                            for (int j = 0; j < num; j++)
                            {
                                in.read((char*)&meshs.back().faces[i].vertex_index[j], sizeof(int));
                            }
                        }
                    }
                }
                else
                    break;

                pNum = 0;
                in.read((char*)&pNum, sizeof(int));
                if (pNum > 0)
                {
                    for (int i = 0; i < pNum; ++i)
                    {
                        int num = 0;
                        in.read((char*)&num, sizeof(int));
                        if (num == 3)
                        {
                            std::vector<float> v(3);
                            for (int j = 0; j < num; j++)
                            {
                                in.read((char*)&v[j], sizeof(float));
                            }
                            meshs.back().vertices.push_back(MeshVertex(Point3(MM2INT(v[0]), MM2INT(v[1]), MM2INT(v[02]))));
                        }
                    }
                }
                meshs.back().finish();
            }
        }
        in.close();
    }

    void getPaintSupport(SliceDataStorage& storage, SliceContext* application, const int layer_thickness, const int slice_layer_count, const bool use_variable_layer_heights)
    {
        std::vector<cura52::Mesh> supportMesh;
        getBinaryData(application->supportFile(), supportMesh, application->currentGroup()->settings);
        if (!supportMesh.empty())
        {
            for (Mesh& mesh : supportMesh)
            {
                SlicedData slicedData;
                mesh.settings.add("support_paint_enable", "true");
                mesh.settings.add("keep_open_polygons", "true");
                mesh.settings.add("minimum_polygon_circumference", "0.05");//����С�ڴ�ֵ�ᱻ�Ƴ�
                sliceMesh(application, &mesh, layer_thickness, slice_layer_count, use_variable_layer_heights, nullptr, slicedData);
                handleSupportModifierMesh(storage, mesh.settings, &slicedData);
            }
        }
    }

    void getPaintSupport_anti(SliceDataStorage& storage, SliceContext* application, const int layer_thickness, const int slice_layer_count, const bool use_variable_layer_heights)
    {
        std::vector<cura52::Mesh> antiMesh;
        getBinaryData(application->antiSupportFile(), antiMesh, application->currentGroup()->settings);
        if (!antiMesh.empty())
        {
            for (Mesh& mesh : antiMesh)
            {
                SlicedData slicedData;
                mesh.settings.add("support_paint_enable", "true");
                mesh.settings.add("keep_open_polygons", "true");
                mesh.settings.add("minimum_polygon_circumference", "0.05");//����С�ڴ�ֵ�ᱻ�Ƴ�
                mesh.settings.add("anti_overhang_mesh", "true");
                sliceMesh(application, &mesh, layer_thickness, slice_layer_count, use_variable_layer_heights, nullptr, slicedData);
                handleSupportModifierMesh(storage, mesh.settings, &slicedData);
            }
        }
    }

    void getZseamLine(SliceDataStorage& storage, SliceContext* application, const int layer_thickness, const int slice_layer_count, const bool use_variable_layer_heights)
    {
        std::vector<cura52::Mesh> ZseamMesh;
        getBinaryData(application->seamFile(), ZseamMesh, application->currentGroup()->settings);
        if (!ZseamMesh.empty())
        {
            storage.zSeamPoints.resize(slice_layer_count);
            for (Mesh& mesh : ZseamMesh)
            {
                SlicedData slicedData;
                std::vector<SlicerLayer> Zseamlineslayers;

                mesh.settings.add("zseam_paint_enable", "true");
                sliceMesh(application, &mesh, layer_thickness, slice_layer_count, use_variable_layer_heights, nullptr, Zseamlineslayers);
                for (unsigned int layer_nr = 0; layer_nr < Zseamlineslayers.size(); layer_nr++)
                {
                    for (size_t i = 0; i < Zseamlineslayers[layer_nr].segments.size(); i++)
                    {
                        storage.zSeamPoints[layer_nr].ZseamLayers.push_back(ZseamDrawPoint(Zseamlineslayers[layer_nr].segments[i].start));
                    }
                }
            }
        }
    }
    void getZseamLine_anti(SliceDataStorage& storage, SliceContext* application, const int layer_thickness, const int slice_layer_count, const bool use_variable_layer_heights)
    {
        std::vector<cura52::Mesh> antiMesh;
        getBinaryData(application->antiSeamFile(), antiMesh, application->currentGroup()->settings);
        if (!antiMesh.empty())
        {
            storage.interceptSeamPoints.resize(slice_layer_count);
            for (Mesh& mesh : antiMesh)
            {
                SlicedData slicedData;
                std::vector<SlicerLayer> Zseamlineslayers;

                mesh.settings.add("zseam_paint_enable", "true");
                sliceMesh(application, &mesh, layer_thickness, slice_layer_count, use_variable_layer_heights, nullptr, Zseamlineslayers);
                for (unsigned int layer_nr = 0; layer_nr < Zseamlineslayers.size(); layer_nr++)
                {
                    for (size_t i = 0; i < Zseamlineslayers[layer_nr].segments.size(); i++)
                    {
                        storage.interceptSeamPoints[layer_nr].ZseamLayers.push_back(ZseamDrawPoint(Zseamlineslayers[layer_nr].segments[i].start));
                    }
                }
            }
        }
    }
}


