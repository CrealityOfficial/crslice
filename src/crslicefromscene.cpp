#include "crslicefromscene.h"
#include "ccglobal/log.h"
#include "crgroup.h"
#include "Application.h"
#include "ExtruderTrain.h"
#include "FffProcessor.h"
#include "Slice.h"
#include "communication/CommandLine.h"
namespace crslice
{
    static void trimesh2CuraMesh(TriMeshPtr mesh, cura::Mesh *curaMesh)
    {
        for (size_t i = 0; i < mesh->faces.size(); i++)
        {
            //const cura::FMatrix4x3 transformation ;//= last_settings->get<FMatrix4x3>("mesh_rotation_matrix"); // The transformation applied to the model when loaded.
            //loadMeshIntoMeshGroup(&slice.scene.mesh_groups[mesh_group_index], argument.c_str(), transformation, last_extruder->settings)
            const auto &face = mesh->faces[i];
            cura::Point3 v0(mesh->vertices[face[0]].x, mesh->vertices[face[0]].y, mesh->vertices[face[0]].z);
            cura::Point3 v1(mesh->vertices[face[1]].x, mesh->vertices[face[1]].y, mesh->vertices[face[1]].z);
            cura::Point3 v2(mesh->vertices[face[2]].x, mesh->vertices[face[2]].y, mesh->vertices[face[2]].z);
            curaMesh->addFace(v0, v1, v2);
        }
        curaMesh->finish();
    }
    static void crSetting2CuraSettings(SettingsPtr crSettings, cura::Settings *curaSettings)
    {
        for (const std::pair<std::string, std::string> pair : crSettings->settings)
        {
             curaSettings->add(pair.first, pair.second);
        }

    }
    CRSliceFromScene::CRSliceFromScene(CrScenePtr scene, const SliceInitCfg sliceCfg)
        : m_sliced(false)
        , m_scene(scene)
        , m_sliceInitCfg(sliceCfg)
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
    void CRSliceFromScene::sendCurrentPosition(const cura::Point&)
    {
    }
    void CRSliceFromScene::sendFinishedSlicing() const
    {
    }
    void CRSliceFromScene::sendLayerComplete(const cura::LayerIndex&, const cura::coord_t&, const cura::coord_t&)
    {
    }

    void CRSliceFromScene::sendLineTo(const cura::PrintFeatureType&, const cura::Point&, const cura::coord_t&, const cura::coord_t&, const cura::Velocity&)
    {
    }

    void CRSliceFromScene::sendOptimizedLayerData()
    {
    }

    void CRSliceFromScene::sendPolygon(const cura::PrintFeatureType&, const cura::ConstPolygonRef&, const cura::coord_t&, const cura::coord_t&, const cura::Velocity&)
    {
    }

    void CRSliceFromScene::sendPolygons(const cura::PrintFeatureType&, const cura::Polygons&, const cura::coord_t&, const cura::coord_t&, const cura::Velocity&)
    {
    }

    void CRSliceFromScene::setExtruderForSend(const cura::ExtruderTrain&)
    {
    }

    void CRSliceFromScene::setLayerForSend(const cura::LayerIndex&)
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
        if (m_scene == nullptr)
            return;
       // cura::FffProcessor cura::FffProcessor::instance;
        cura::FffProcessor::getInstance()->time_keeper.restart();
#if 1
        // Count the number of mesh groups to slice for.
       size_t num_mesh_groups = m_scene->m_groups.size();
       if (num_mesh_groups == 0)
           return;
        cura::Slice slice(num_mesh_groups);

        cura::Application::getInstance().current_slice = &slice;

        
        slice.scene.extruders.emplace_back(0, &slice.scene.settings); // Always have one extruder.
        cura::ExtruderTrain* last_extruder = &slice.scene.extruders[0];
        cura::Settings* last_settings = &slice.scene.settings;

        cura::CommandLine cmdline;
        cmdline.loadJSON(m_sliceInitCfg.initCfgFile, slice.scene.settings);
        cura::Application::getInstance().setSliceCommunication(&cmdline);
        cmdline.setsliceHandler(&slice);
        crSetting2CuraSettings(m_scene->m_settings, &(slice.scene.settings));
        for (size_t mesh_group_index = 0; mesh_group_index < num_mesh_groups; mesh_group_index++)
        {
            cura::FMatrix4x3 transformation;// = last_settings->get<FMatrix4x3>("mesh_rotation_matrix"); // The transformation applied to the model when loaded.
            CrGroup* groupPtr = m_scene->getGroupsIndex(mesh_group_index);
            crSetting2CuraSettings(groupPtr->m_settings, &(slice.scene.mesh_groups[mesh_group_index].settings));
            for (auto obj : groupPtr->m_objects)
            {
                if (obj.m_mesh.get() != nullptr)
                {
                    cura::Mesh* meshOut = new cura::Mesh(last_extruder->settings);
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
#else
        cura::Slice slice(1);
        cura::Settings* last_settings = &slice.scene.settings;
        slice.scene.extruders.emplace_back(0, &slice.scene.settings);
        cura::ExtruderTrain* last_extruder = &slice.scene.extruders[0];

        cura::CommandLine cmdline;
        cmdline.setsliceHandler(&slice);
        cura::Application::getInstance().current_slice = &slice;
        cmdline.loadJSON(m_sliceInitCfg.initCfgFile, slice.scene.settings);
        cura::Application::getInstance().setSliceCommunication(&cmdline);

        const cura::FMatrix4x3 transformation = last_settings->get<cura::FMatrix4x3>("mesh_rotation_matrix"); // The transformation applied to the model when loaded.

        last_extruder->settings.add("extruder_nr", std::to_string(last_extruder->extruder_nr));

        if (!cura::loadMeshIntoMeshGroup(&slice.scene.mesh_groups[0], m_sliceInitCfg.testMeshFile.c_str(), transformation, last_extruder->settings))
        {
            LOGD("load test mesh error");
            exit(-1);
        }


#endif
        if (m_sliceInitCfg.gcodeOutFile.empty())
        {
            LOGE("should set gcodeOutFile!!!!!");
        }
         cura::FffProcessor::getInstance()->setTargetFile(m_sliceInitCfg.gcodeOutFile.c_str());
        // Start slicing.
        cura::Application::getInstance().startThreadPool(); // Start the thread pool

        slice.scene.mesh_groups[0].finalize();

        slice.compute();
        // Finalize the processor. This adds the end g-code and reports statistics.
        cura::FffProcessor::getInstance()->finalize();

        m_sliced = true;
    }

} // namespace crslice