#ifndef CRSLICE_FROM_SCENE_COMMANDLINE_H
#define CRSLICE_FROM_SCENE_COMMANDLINE_H
#include "crslice/crscene.h"
#include "communication/Communication.h"

namespace crslice
{
    class CRSliceFromScene : public cura::Communication
    {
    public:
        CRSliceFromScene(CrScenePtr scene);
        virtual ~CRSliceFromScene();

        void beginGCode() override;
        void flushGCode() override;

        bool isSequential() const override;
        bool hasSlice() const override;

        void sendCurrentPosition(const cura::Point&) override;
        void sendFinishedSlicing() const override;

        void sendGCodePrefix(const std::string&) const override;
        void sendSliceUUID(const std::string& slice_uuid) const override;

        void sendLayerComplete(const cura::LayerIndex&, const cura::coord_t&, const cura::coord_t&) override;
        void sendLineTo(const cura::PrintFeatureType&, const cura::Point&, const cura::coord_t&, const cura::coord_t&, const cura::Velocity&) override;

        void sendOptimizedLayerData() override;
        void sendPolygon(const cura::PrintFeatureType&, const cura::ConstPolygonRef&, const cura::coord_t&, const cura::coord_t&, const cura::Velocity&) override;
        void sendPolygons(const cura::PrintFeatureType&, const cura::Polygons&, const cura::coord_t&, const cura::coord_t&, const cura::Velocity&) override;

        void sendPrintTimeMaterialEstimates() const override;
        void sendProgress(const float& progress) const override;

        void setExtruderForSend(const cura::ExtruderTrain&) override;
        void setLayerForSend(const cura::LayerIndex&) override;

        void sliceNext() override;
    protected:
        void runFinished();
    private:
        bool m_sliced;
        CrScenePtr m_scene;
    };

} //namespace crslice

#endif //CRSLICE_FROM_SCENE_COMMANDLINE_H