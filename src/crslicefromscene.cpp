#include "crslicefromscene.h"
#include "ccglobal/log.h"
#include "crgroup.h"
#include "cura5.0/include/Application.h"
#include "cura5.0/include/ExtruderTrain.h"
#include "cura5.0/include/FffProcessor.h"
#include "cura5.0/include/Slice.h"
#include "cura5.0/include/utils/Coord_t.h"
#include "crcommon/jsonloader.h"
#include "cxutil/slicer/ExtruderTrain.h"
#include "cura-cloud/Slice.h"
#include "cura-cloud/FffProcessor.h"
namespace crslice
{
    static void trimesh2CuraMesh(TriMeshPtr mesh, cura52::Mesh& curaMesh)
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
        }
        curaMesh.finish();
    }
    static void trimesh2CuraMesh(TriMeshPtr mesh, cxutil::Mesh& curaMesh)
    {
        curaMesh.faces.reserve(mesh->faces.size());
        curaMesh.vertices.reserve(mesh->vertices.size());

        for (size_t i = 0; i < mesh->faces.size(); i++)
        {
            const trimesh::TriMesh::Face& face = mesh->faces[i];
            const trimesh::vec3& v1 = mesh->vertices[face[0]];
            const trimesh::vec3& v2 = mesh->vertices[face[1]];
            const trimesh::vec3& v3 = mesh->vertices[face[2]];

            cxutil::Point3 p1(MM2INT(v1.x), MM2INT(v1.y), MM2INT(v1.z));
            cxutil::Point3 p2(MM2INT(v2.x), MM2INT(v2.y), MM2INT(v2.z));
            cxutil::Point3 p3(MM2INT(v3.x), MM2INT(v3.y), MM2INT(v3.z));
            curaMesh.addFace(p1, p2, p3);
        }
        curaMesh.finish();
    }

    static void crSetting2CuraSettings(const crcommon::Settings &crSettings, cura52::Settings *curaSettings)
    {
        for (const std::pair<std::string, std::string> pair : crSettings.settings)
        {
             curaSettings->add(pair.first, pair.second);
        }

    }
    static void crSetting2CuraSettings(const crcommon::Settings &crSettings, cxutil::Settings *curaSettings)
    {
        for (const std::pair<std::string, std::string> pair : crSettings.settings)
        {
             curaSettings->add(pair.first, pair.second);
        }

    }

    CRSliceFromScene::CRSliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer)
        : m_sliced(false)
        , m_scene(scene)
        , m_tracer(tracer)
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
        if (m_tracer)
            m_tracer->progress(progress);
    }

    void CRSliceFromScene::sliceNext()
    {
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

        size_t numGroup = m_scene->m_groups.size();
        assert(numGroup > 0);
        cura52::Slice slice(numGroup);

        cura52::Application::getInstance().current_slice = &slice;

        bool sliceValible = false;

        std::vector<SettingsPtr>& extruderKVs = m_scene->m_extruders;
        int extruder_nr = 0;
        for (SettingsPtr settings : extruderKVs)
        {
            slice.scene.extruders.emplace_back(extruder_nr, &slice.scene.settings);
            cura52::ExtruderTrain& extruder = slice.scene.extruders[extruder_nr];
            extruder.settings.settings.swap(settings->settings);
            ++extruder_nr;
        }

        if(slice.scene.extruders.size() == 0)
            slice.scene.extruders.emplace_back(0, &slice.scene.settings); // Always have one extruder.

        crSetting2CuraSettings(*(m_scene->m_settings), &(slice.scene.settings));
        for (size_t i = 0; i < numGroup; i++)
        {
            CrGroup* crGroup = m_scene->getGroupsIndex(i);
            if (crGroup)
            {
                crSetting2CuraSettings(*(crGroup->m_settings), &(slice.scene.mesh_groups[i].settings));
                for (const CrObject& object : crGroup->m_objects)
                {
                    if (object.m_mesh.get() != nullptr)
                    {
                        slice.scene.mesh_groups[i].meshes.emplace_back(cura52::Mesh());
                        cura52::Mesh& mesh = slice.scene.mesh_groups[i].meshes.back();
                        trimesh2CuraMesh(object.m_mesh, mesh);
                        crSetting2CuraSettings(*(object.m_settings), &(mesh.settings));
                        sliceValible = true;
                    }
                }
            }
        }

        if (sliceValible == false)
        {
            LOGE("scene convert failed.");
            runFinished();
            return;
        }

        slice.finalize();
        slice.compute();
        // Finalize the processor. This adds the end g-code and reports statistics.
        cura52::FffProcessor::getInstance()->finalize();

        runFinished();
    }

    void CRSliceFromScene::runFinished()
    {
        m_sliced = true;
    }
    ///////////////////////////////////////////////////////////////////////
    CRSliceFromScene480::CRSliceFromScene480(CrScenePtr scene, ccglobal::Tracer* tracer)
        : m_sliced(false)
        , m_scene(scene)
        , m_tracer(tracer)
    {

    }

    CRSliceFromScene480::~CRSliceFromScene480()
    {

    }

    // These are not applicable to command line slicing.
    void CRSliceFromScene480::beginGCode()
    {
    }

    void CRSliceFromScene480::flushGCode()
    {
    }

    void CRSliceFromScene480::sendCurrentPosition(const cxutil::Point&)
    {
    }

    void CRSliceFromScene480::sendFinishedSlicing() const
    {
    }

    void CRSliceFromScene480::sendLayerComplete(const cxutil::LayerIndex&, const cxutil::coord_t&, const cxutil::coord_t&)
    {
    }

    void CRSliceFromScene480::sendLineTo(const cxutil::PrintFeatureType&, const cxutil::Point&, const cxutil::coord_t&, const cxutil::coord_t&, const cxutil::Velocity&)
    {
    }

    void CRSliceFromScene480::sendOptimizedLayerData()
    {
    }

    void CRSliceFromScene480::sendPolygon(const cxutil::PrintFeatureType& type, const cxutil::ConstPolygonRef& polygon, const cxutil::coord_t& line_width, const cxutil::coord_t& line_thickness, const cxutil::Velocity& velocity)
    {
    }

    void CRSliceFromScene480::sendPolygons(const cxutil::PrintFeatureType&, const cxutil::Polygons&, const cxutil::coord_t&, const cxutil::coord_t&, const cxutil::Velocity&)
    {
    }

    void CRSliceFromScene480::setExtruderForSend(const cxutil::ExtruderTrain&)
    {
    }

    void CRSliceFromScene480::setLayerForSend(const cxutil::LayerIndex&)
    {
    }

    bool CRSliceFromScene480::hasSlice() const
    {
        return !m_sliced;
    }

    bool CRSliceFromScene480::isSequential() const
    {
        return true;
    }

    void CRSliceFromScene480::sendGCodePrefix(const std::string&) const
    {
    }

    void CRSliceFromScene480::sendSliceUUID(const std::string& slice_uuid) const
    {
    }

    void CRSliceFromScene480::sendPrintTimeMaterialEstimates() const
    {
    }

    void CRSliceFromScene480::sendProgress(const float& progress) const
    {
        if (m_tracer)
            m_tracer->progress(progress);
    }

    void CRSliceFromScene480::sliceNext()
    {
#if 1
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

        size_t numGroup = m_scene->m_groups.size();
        assert(numGroup > 0);
        cxutil::Slice slice(numGroup);

       // cxutil::Application::getInstance().current_slice = &slice;
        cxutil::FffProcessor processor;
        processor.setTargetFile(outputFile.c_str());
        processor.time_keeper.restart();
        processor.gcode_writer.scene = &slice.scene;
        processor.gcode_writer.gcode.scene = &slice.scene;
        processor.polygon_generator.scene = &slice.scene;
        processor.gcode_writer.layer_plan_buffer.scene = &slice.scene;
        processor.gcode_writer.layer_plan_buffer.preheat_config.scene = &slice.scene;

        bool sliceValible = false;

        std::vector<SettingsPtr>& extruderKVs = m_scene->m_extruders;
        int extruder_nr = 0;
        for (SettingsPtr settings : extruderKVs)
        {
            slice.scene.extruders.emplace_back(extruder_nr, &slice.scene.settings);
            cxutil::ExtruderTrain& extruder = slice.scene.extruders[extruder_nr];
            extruder.settings->settings.swap(settings->settings);
            ++extruder_nr;
        }

        if (slice.scene.extruders.size() == 0)
            slice.scene.extruders.emplace_back(0, &slice.scene.settings); // Always have one extruder.

        crSetting2CuraSettings(*(m_scene->m_settings), &(slice.scene.settings));
        for (size_t i = 0; i < numGroup; i++)
        {
            CrGroup* crGroup = m_scene->getGroupsIndex(i);
            if (crGroup)
            {
                crSetting2CuraSettings(*(crGroup->m_settings), (slice.scene.mesh_groups[i].settings));
                for (const CrObject& object : crGroup->m_objects)
                {
                    if (object.m_mesh.get() != nullptr)
                    {
                        slice.scene.mesh_groups[i].meshes.emplace_back(new cxutil::Mesh());
                        cxutil::Mesh* mesh = slice.scene.mesh_groups[i].meshes.back();
                        trimesh2CuraMesh(object.m_mesh, *mesh);
                        crSetting2CuraSettings(*(object.m_settings), (mesh->settings));
                        sliceValible = true;
                    }
                }
            }
        }

        if (sliceValible == false)
        {
            LOGE("scene convert failed.");
            runFinished();
            return;
        }

        slice.finalize();
        slice.compute();
        // Finalize the processor. This adds the end g-code and reports statistics.
        processor.finalize();

        runFinished();
#endif
    }

    void CRSliceFromScene480::runFinished()
    {
        m_sliced = true;
    }
} // namespace crslice