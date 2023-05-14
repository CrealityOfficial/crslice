#ifndef CRSLICE_FROM_SCENE_COMMANDLINE_CACHE_DEBUGGER_H
#define CRSLICE_FROM_SCENE_COMMANDLINE_CACHE_DEBUGGER_H
#include "crslice/crscene.h"
#include "Slice.h"
#include "communication/Communication.h"
#include "debugger.h"

namespace crslice
{
    struct GroupData
    {
        cura52::Point3 _min;
        cura52::Point3 _max;
    };

    class CacheDebugger : public cura52::Communication
        , public cura52::Debugger
    {
    public:
        CacheDebugger(cura52::Application* _application, CrScenePtr scene);
        virtual ~CacheDebugger();

        cura52::Slice* createSlice() override;
        bool hasSlice() const override;

    private:
        bool m_haveSlice;
        CrScenePtr m_scene;

        cura52::Application* application;

    public:
        void startSlice(int count) override;
        void startGroup(int index) override;
        void groupBox(const cura52::Point3& _min, const cura52::Point3& _max) override;

    public:
        std::vector<GroupData> groupDatas;
        int currentIndex;

        GroupData groupData;
    };

} //namespace crslice

#endif //CRSLICE_FROM_SCENE_COMMANDLINE_CACHE_DEBUGGER_H