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
} // namespace crslice