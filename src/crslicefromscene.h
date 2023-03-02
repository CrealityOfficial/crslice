#ifndef CRSLICE_FROM_SCENE_COMMANDLINE_H
#define CRSLICE_FROM_SCENE_COMMANDLINE_H
#include "crslice/crscene.h"
#include "communication/Communication.h"

namespace crslice
{
    class CRSliceFromScene : public cura52::Communication
    {
    public:
        CRSliceFromScene(CrScenePtr scene);
        virtual ~CRSliceFromScene();

        void beginGCode() override;
        void flushGCode() override;

        bool isSequential() const override;
        bool hasSlice() const override;

        void sendCurrentPosition(const cura52::Point&) override;
        void sendFinishedSlicing() const override;

        void sendGCodePrefix(const std::string&) const override;
        void sendSliceUUID(const std::string& slice_uuid) const override;

        void sendLayerComplete(const cura52::LayerIndex&, const cura52::coord_t&, const cura52::coord_t&) override;
        void sendLineTo(const cura52::PrintFeatureType&, const cura52::Point&, const cura52::coord_t&, const cura52::coord_t&, const cura52::Velocity&) override;

        void sendOptimizedLayerData() override;
        void sendPolygon(const cura52::PrintFeatureType&, const cura52::ConstPolygonRef&, const cura52::coord_t&, const cura52::coord_t&, const cura52::Velocity&) override;
        void sendPolygons(const cura52::PrintFeatureType&, const cura52::Polygons&, const cura52::coord_t&, const cura52::coord_t&, const cura52::Velocity&) override;

        void sendPrintTimeMaterialEstimates() const override;
        void sendProgress(const float& progress) const override;

        void setExtruderForSend(const cura52::ExtruderTrain&) override;
        void setLayerForSend(const cura52::LayerIndex&) override;

        void sliceNext() override;
    protected:
        void runFinished();
    private:
        bool m_sliced;
        CrScenePtr m_scene;
    };

} //namespace crslice

#endif //CRSLICE_FROM_SCENE_COMMANDLINE_H