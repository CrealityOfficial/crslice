#ifndef CRSLICE_FROM_SCENE_COMMANDLINE_H
#define CRSLICE_FROM_SCENE_COMMANDLINE_H
#include "crslice/crscene.h"
#include "Slice.h"
#include "communication/Communication.h"

namespace crslice
{
    class CRSliceFromScene : public cura52::Communication
    {
    public:
        CRSliceFromScene(cura52::Application* _application, CrScenePtr scene);
        virtual ~CRSliceFromScene();

        cura52::Slice* createSlice() override;
        bool hasSlice() const override;

    private:
        bool m_haveSlice;
        CrScenePtr m_scene;

        cura52::Application* application;
    };

} //namespace crslice

#endif //CRSLICE_FROM_SCENE_COMMANDLINE_H