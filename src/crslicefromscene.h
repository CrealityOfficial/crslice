#ifndef CRSLICE_FROM_SCENE_COMMANDLINE_H
#define CRSLICE_FROM_SCENE_COMMANDLINE_H
#include "crslice/crscene.h"
#include "cura5.0/include/communication/Communication.h"
#include "cura-cloud/communication/Communication.h"

namespace crslice
{
    class CRSliceFromScene : public cura52::Communication
    {
    public:
        CRSliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer = nullptr);
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

        ccglobal::Tracer* m_tracer;
    };

    class CRSliceFromScene480 : public cxutil::Communication
    {
    public:
        CRSliceFromScene480(CrScenePtr scene, ccglobal::Tracer* tracer = nullptr);
        virtual ~CRSliceFromScene480();

        void beginGCode() override;
        void flushGCode() override;

        bool isSequential() const override;
        bool hasSlice() const override;

        void sendCurrentPosition(const cxutil::Point&) override;
        void sendFinishedSlicing() const override;

        void sendGCodePrefix(const std::string&) const override;
        void sendSliceUUID(const std::string& slice_uuid) const ;

        void sendLayerComplete(const cxutil::LayerIndex&, const cxutil::coord_t&, const cxutil::coord_t&) override;
        void sendLineTo(const cxutil::PrintFeatureType&, const cxutil::Point&, const cxutil::coord_t&, const cxutil::coord_t&, const cxutil::Velocity&) override;

        void sendOptimizedLayerData() override;
        void sendPolygon(const cxutil::PrintFeatureType& type, const cxutil::ConstPolygonRef& polygon, const cxutil::coord_t& line_width, const cxutil::coord_t& line_thickness, const cxutil::Velocity& velocity) override;
        void sendPolygons(const cxutil::PrintFeatureType&, const cxutil::Polygons&, const cxutil::coord_t&, const cxutil::coord_t&, const cxutil::Velocity&) override;

        void sendPrintTimeMaterialEstimates() const override;
        void sendProgress(const float& progress) const override;

        void setExtruderForSend(const cxutil::ExtruderTrain&) override;
        void setLayerForSend(const cxutil::LayerIndex&) override;

        void sliceNext() override;
    protected:
        void runFinished();
    private:
        bool m_sliced;
        CrScenePtr m_scene;

        ccglobal::Tracer* m_tracer;
    };
} //namespace crslice

#endif //CRSLICE_FROM_SCENE_COMMANDLINE_H