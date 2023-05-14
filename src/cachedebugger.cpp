#include "cachedebugger.h"
#include "ccglobal/log.h"
#include "create.h"

namespace crslice
{
    CacheDebugger::CacheDebugger(cura52::Application* _application, CrScenePtr scene)
        : m_haveSlice(true)
        , m_scene(scene)
        , application(_application)
        , currentIndex(0)
    {

    }

    CacheDebugger::~CacheDebugger()
    {
    
    }

    bool CacheDebugger::hasSlice() const
    {
        return m_haveSlice;
    }

    cura52::Slice* CacheDebugger::createSlice()
    {
        cura52::Slice* slice = createSliceFromCrScene(application, m_scene);
        m_scene->release();
        m_haveSlice = false;
        return slice;
    }

    void CacheDebugger::startSlice(int count)
    {
        groupDatas.resize(count);
    }

    void CacheDebugger::startGroup(int index)
    {
        currentIndex = index;
    }

    void CacheDebugger::groupBox(const cura52::Point3& _min, const cura52::Point3& _max)
    {
        groupData._max = _max;
        groupData._min = _min;
    }

    void CacheDebugger::sliceLayerCount(int count)
    {
        groupData.sliceLayers.resize(count);
        groupData.partLayers.resize(count);
    }

    void CacheDebugger::sliceLayerData(int index, int z, const cura52::Polygons& polygons, const cura52::Polygons& openPolygons)
    {
        SlicerLayerData& data = groupData.sliceLayers.at(index);
        data.z = z;
        data.polygons = polygons;
        data.openPolygons = openPolygons;
    }

    void CacheDebugger::parts(const cura52::SliceMeshStorage& storage)
    {
        for (int i = 0; i < (int)storage.layers.size(); ++i)
        {
            const cura52::SliceLayer& layer = storage.layers.at(i);
            LayerPartsData& data = groupData.partLayers.at(i);
            data.z = layer.printZ;

            for(const cura52::SliceLayerPart& p : layer.parts)
                data.parts.push_back(p.outline);
        }
    }
} // namespace crslice