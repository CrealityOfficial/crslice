#include "crslicefromscene.h"
#include "ccglobal/log.h"
#include "crgroup.h"
#include "Application.h"
#include "ExtruderTrain.h"
#include "FffProcessor.h"
#include "Slice.h"
#include "communication/CommandLine.h"
#include "utils/Coord_t.h"
namespace crslice
{
    static void trimesh2CuraMesh(TriMeshPtr mesh, cura52::Mesh *curaMesh)
    {
        curaMesh->faces.reserve(mesh->faces.size());
        curaMesh->vertices.reserve(mesh->vertices.size());

        for (size_t i = 0; i < mesh->faces.size(); i++)
        {
            //const cura::FMatrix4x3 transformation ;//= last_settings->get<FMatrix4x3>("mesh_rotation_matrix"); // The transformation applied to the model when loaded.
            //loadMeshIntoMeshGroup(&slice.scene.mesh_groups[mesh_group_index], argument.c_str(), transformation, last_extruder->settings)
            const auto &face = mesh->faces[i];
            cura52::Point3 v0(MM2INT(mesh->vertices[face[0]].x), MM2INT(mesh->vertices[face[0]].y), MM2INT(mesh->vertices[face[0]].z));
            cura52::Point3 v1(MM2INT(mesh->vertices[face[1]].x), MM2INT(mesh->vertices[face[1]].y), MM2INT(mesh->vertices[face[1]].z));
            cura52::Point3 v2(MM2INT(mesh->vertices[face[2]].x), MM2INT(mesh->vertices[face[2]].y), MM2INT(mesh->vertices[face[2]].z));
            curaMesh->addFace(v0, v1, v2);
        }
        curaMesh->finish();
    }
    static void crSetting2CuraSettings(SettingsPtr crSettings, cura52::Settings *curaSettings)
    {
        for (const std::pair<std::string, std::string> pair : crSettings->settings)
        {
             curaSettings->add(pair.first, pair.second);
        }

    }
    CRSliceFromScene::CRSliceFromScene(CrScenePtr scene)
        : m_sliced(false)
        , m_scene(scene)
    {

    }
    CRSliceFromScene::~CRSliceFromScene()
    {
    
    }
    // These are not applicable to command line slicing.
    void CRSliceFromScene::beginGCode()
    {
    }
    void CRSliceFromScene::flushGCode()
    {
    }
    void CRSliceFromScene::sendCurrentPosition(const cura52::Point&)
    {
    }
    void CRSliceFromScene::sendFinishedSlicing() const
    {
    }
    void CRSliceFromScene::sendLayerComplete(const cura52::LayerIndex&, const cura52::coord_t&, const cura52::coord_t&)
    {
    }

    void CRSliceFromScene::sendLineTo(const cura52::PrintFeatureType&, const cura52::Point&, const cura52::coord_t&, const cura52::coord_t&, const cura52::Velocity&)
    {
    }

    void CRSliceFromScene::sendOptimizedLayerData()
    {
    }

    void CRSliceFromScene::sendPolygon(const cura52::PrintFeatureType&, const cura52::ConstPolygonRef&, const cura52::coord_t&, const cura52::coord_t&, const cura52::Velocity&)
    {
    }

    void CRSliceFromScene::sendPolygons(const cura52::PrintFeatureType&, const cura52::Polygons&, const cura52::coord_t&, const cura52::coord_t&, const cura52::Velocity&)
    {
    }

    void CRSliceFromScene::setExtruderForSend(const cura52::ExtruderTrain&)
    {
    }

    void CRSliceFromScene::setLayerForSend(const cura52::LayerIndex&)
    {
    }

    bool CRSliceFromScene::hasSlice() const
    {
        return !m_sliced;
    }

    bool CRSliceFromScene::isSequential() const
    {
        return true;
    }

    void CRSliceFromScene::sendGCodePrefix(const std::string&) const
    {
    }

    void CRSliceFromScene::sendSliceUUID(const std::string& slice_uuid) const
    {
    }

    void CRSliceFromScene::sendPrintTimeMaterialEstimates() const
    {
    }

    void CRSliceFromScene::sendProgress(const float& progress) const
    {
    }

    void CRSliceFromScene::sliceNext()
    {
        bool sliceValible = false;
        if (m_scene == nullptr || !m_scene->valid())
        {
            LOGE("input scene is empty or invalid.");
            runFinished();
            return;
        }

        std::string outputFile = m_scene->m_gcodeFileName;
        if (outputFile.empty())
        {
            LOGE("output file name is not set.");
            runFinished();
            return;
        }
        cura52::FffProcessor::getInstance()->setTargetFile(outputFile.c_str());
        cura52::FffProcessor::getInstance()->time_keeper.restart();

        size_t num_mesh_groups = m_scene->m_groups.size();
        assert(num_mesh_groups > 0);
        cura52::Slice slice(num_mesh_groups);

        cura52::Application::getInstance().current_slice = &slice;

        
        slice.scene.extruders.emplace_back(0, &slice.scene.settings); // Always have one extruder.
        cura52::ExtruderTrain* last_extruder = &slice.scene.extruders[0];
        cura52::Settings* last_settings = &slice.scene.settings;

        cura52::CommandLine cmdline;
        //cmdline.loadJSON(m_sliceInitCfg.initCfgFile, slice.scene.settings);
        cura52::Application::getInstance().setSliceCommunication(&cmdline);
        cmdline.setsliceHandler(&slice);
        crSetting2CuraSettings(m_scene->m_settings, &(slice.scene.settings));
        for (size_t mesh_group_index = 0; mesh_group_index < num_mesh_groups; mesh_group_index++)
        {
            cura52::FMatrix4x3 transformation;// = last_settings->get<FMatrix4x3>("mesh_rotation_matrix"); // The transformation applied to the model when loaded.
            CrGroup* groupPtr = m_scene->getGroupsIndex(mesh_group_index);
            crSetting2CuraSettings(groupPtr->m_settings, &(slice.scene.mesh_groups[mesh_group_index].settings));
            for (auto obj : groupPtr->m_objects)
            {
                if (obj.m_mesh.get() != nullptr)
                {
                    cura52::Mesh* meshOut = new cura52::Mesh(last_extruder->settings);
                    trimesh2CuraMesh(obj.m_mesh, meshOut);
                    crSetting2CuraSettings(obj.m_settings, &(meshOut->settings));
                    slice.scene.mesh_groups[mesh_group_index].meshes.emplace_back(*meshOut);
                    sliceValible = true;

                }
            }

        }

        last_extruder->settings.add("extruder_nr", std::to_string(last_extruder->extruder_nr));
        if (sliceValible == false)
            return;

        for (size_t mesh_group_index = 0; mesh_group_index < num_mesh_groups; mesh_group_index++)
        {
            slice.scene.mesh_groups[mesh_group_index].finalize();

        }

        slice.scene.mesh_groups[0].finalize();

        slice.compute();
        // Finalize the processor. This adds the end g-code and reports statistics.
        cura52::FffProcessor::getInstance()->finalize();

        runFinished();
    }

    void CRSliceFromScene::runFinished()
    {
        m_sliced = true;
    }

} // namespace crslice