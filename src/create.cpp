#include "create.h"
#include "Application.h"

#include "crgroup.h"
#include "ccglobal/log.h"
#include "mmesh/util/trimecr30.h"

namespace crslice
{
    void trimesh2CuraMesh(trimesh::TriMesh* mesh, cura52::Mesh& curaMesh, cura52::Application* application)
    {
        curaMesh.faces.reserve(mesh->faces.size());
        curaMesh.vertices.reserve(mesh->vertices.size());

        for (size_t i = 0; i < mesh->faces.size(); i++)
        {
            const trimesh::TriMesh::Face& face = mesh->faces[i];
            const trimesh::vec3& v1 = mesh->vertices[face[0]];
            const trimesh::vec3& v2 = mesh->vertices[face[1]];
            const trimesh::vec3& v3 = mesh->vertices[face[2]];

            cura52::Point3 p1(MM2INT(v1.x), MM2INT(v1.y), MM2INT(v1.z));
            cura52::Point3 p2(MM2INT(v2.x), MM2INT(v2.y), MM2INT(v2.z));
            cura52::Point3 p3(MM2INT(v3.x), MM2INT(v3.y), MM2INT(v3.z));
            curaMesh.addFace(p1, p2, p3);

            if (i % 10000 == 0)
            {
                INTERRUPT_BREAK("CRSliceFromScene::sliceNext  trimesh2CuraMesh.");
            }
        }

        if (!application->checkInterrupt())
            curaMesh.finish();
    }

    void crSetting2CuraSettings(const crcommon::Settings& crSettings, cura52::Settings* curaSettings)
    {
        for (const std::pair<std::string, std::string> pair : crSettings.settings)
        {
            curaSettings->add(pair.first, pair.second);
        }
    }

	cura52::Slice* createSliceFromCrScene(cura52::Application* application, CrScenePtr scene)
	{
        if (scene == nullptr || !scene->valid())
        {
            LOGE("input scene is empty or invalid.");
            return nullptr;
        }

        std::string outputFile = scene->m_gcodeFileName;
        if (outputFile.empty())
        {
            LOGE("output file name is not set.");
            return nullptr;
        }

        size_t numGroup = scene->m_groups.size();
        assert(numGroup > 0);

        if (numGroup == 0)
        {
            LOGE("numGroup is 0.");
            return nullptr;
        }

        cura52::Slice* slice = new cura52::Slice(numGroup);

        slice->scene.machine_center_is_zero = scene->machine_center_is_zero;
        slice->gcodeFile = outputFile;

        bool sliceValible = false;

        std::vector<SettingsPtr>& extruderKVs = scene->m_extruders;
        int extruder_nr = 0;
        for (SettingsPtr settings : extruderKVs)
        {
            slice->scene.extruders.emplace_back(extruder_nr, &slice->scene.settings);
            cura52::ExtruderTrain& extruder = slice->scene.extruders[extruder_nr];
            extruder.settings.settings.swap(settings->settings);
            ++extruder_nr;
        }

        if (slice->scene.extruders.size() == 0)
            slice->scene.extruders.emplace_back(0, &slice->scene.settings); // Always have one extruder.

        crSetting2CuraSettings(*(scene->m_settings), &(slice->scene.settings));

        //CR30 
        {
            bool machine_is_belt = slice->scene.settings.get<bool>("machine_is_belt");
            mmesh::Cr30Param cr30Param;
            if (machine_is_belt)
            {
                //cr30Param.belt_support_enable = slice.scene.settings.get<bool>("belt_support_enable");
                cr30Param.belt_support_enable = slice->scene.settings.get<bool>("support_enable");
                cr30Param.machine_depth = slice->scene.settings.get<double>("machine_depth");
                cr30Param.machine_width = slice->scene.settings.get<double>("machine_width");
                //cr30Param.support_angle = slice.scene.settings.get<double>("support_angle");//default 45

                //CR30  support
                std::vector<trimesh::TriMesh*> meshs;
                for (size_t i = 0; i < numGroup; i++)
                {
                    CrGroup* crGroup = scene->getGroupsIndex(i);
                    if (crGroup)
                    {
                        for (const CrObject& object : crGroup->m_objects)
                        {
                            meshs.push_back(object.m_mesh.get());
                        }
                    }
                }

                if (!meshs.empty())
                {
                    std::vector<trimesh::TriMesh*> outmeshs = machine_is_belt == true ? mmesh::sliceBelt(meshs, cr30Param, nullptr) : std::vector<trimesh::TriMesh*>(0);
                    slice->scene.settings.add("support_enable", "false");
                    slice->scene.settings.add("machine_belt_offset", std::to_string(cr30Param.beltOffsetX));
                    slice->scene.settings.add("machine_belt_offset_Y", std::to_string(cr30Param.beltOffsetY));
                    if (!outmeshs.empty())
                    {
                        CrGroup* crGroup = numGroup > 0 ? scene->getGroupsIndex(0) : nullptr;
                        if (crGroup)
                        {
                            CrObject& object = crGroup->m_objects[0];
                            for (auto outmesh : outmeshs)
                            {
                                slice->scene.mesh_groups[0].meshes.emplace_back(cura52::Mesh());
                                cura52::Mesh& meshsupport = slice->scene.mesh_groups[0].meshes.back();
                                trimesh2CuraMesh(outmesh, meshsupport, application);
                                INTERRUPT_BREAK("CRSliceFromScene::sliceNext  trimesh2CuraMesh.");

                                SettingsPtr settings(new crcommon::Settings());
                                *settings = *(object.m_settings);
                                settings->add("support_enable", "false");
                                settings->add("support_mesh", "true");
                                settings->add("support_mesh_drop_down", "false");
                                crSetting2CuraSettings(*settings, &(meshsupport.settings));
                            }
                        }
                        for (auto iter = outmeshs.begin(); iter != outmeshs.end(); ++iter)
                        {
                            if (*iter != nullptr)
                            {
                                delete (*iter);
                                (*iter) = nullptr;
                            }
                        }
                        outmeshs.clear();
                    }
                }
            }
            //CR30 end
        }

        for (size_t i = 0; i < numGroup; i++)
        {
            CrGroup* crGroup = scene->getGroupsIndex(i);
            if (crGroup)
            {
                crSetting2CuraSettings(*(crGroup->m_settings), &(slice->scene.mesh_groups[i].settings));
                for (const CrObject& object : crGroup->m_objects)
                {
                    if (object.m_mesh.get() != nullptr)
                    {
                        INTERRUPT_BREAK("CRSliceFromScene::sliceNext");

                        slice->scene.mesh_groups[i].meshes.emplace_back(cura52::Mesh());
                        cura52::Mesh& mesh = slice->scene.mesh_groups[i].meshes.back();
                        trimesh2CuraMesh(object.m_mesh.get(), mesh, application);

                        INTERRUPT_BREAK("CRSliceFromScene::sliceNext  trimesh2CuraMesh.");
                        crSetting2CuraSettings(*(object.m_settings), &(mesh.settings));
                        sliceValible = true;
                    }
                }
            }

            INTERRUPT_BREAK("CRSliceFromScene::sliceNext");
        }

        if (sliceValible == false || application->checkInterrupt())
        {
            LOGE("scene convert failed.");
            delete slice;
            return nullptr;
        }

        return slice;
	}
}