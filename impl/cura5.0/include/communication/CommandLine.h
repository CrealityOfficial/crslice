//  Copyright (c) 2018-2022 Ultimaker B.V.
//  CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef COMMANDLINE_H_520
#define COMMANDLINE_H_520

#include <rapidjson/document.h> //Loading JSON documents to get settings from them.
#include <string> //To store the command line arguments.
#include <unordered_set>
#include <vector> //To store the command line arguments.

#include "Communication.h" //The class we're implementing.
#include "ccglobal/tracer.h"

namespace cura52
{
    /*
     * \brief When slicing via the command line, interprets the command line
     * arguments to initiate a slice.
     */
    class CommandLine : public Communication
    {
    public:
        CommandLine();
        /*
         * \brief Construct a new communicator that interprets the command line to
         * start a slice.
         * \param arguments The command line arguments passed to the application.
         */
        CommandLine(const std::vector<std::string>& arguments, ccglobal::Tracer* tracer = nullptr);

        /*
         * \brief Indicate that we're beginning to send g-code.
         * This does nothing to the command line.
         */
        void beginGCode() override;

        /*
         * \brief Flush all g-code still in the stream into cout.
         */
        void flushGCode() override;

        /*
         * \brief Indicates that for command line output we need to send the g-code
         * from start to finish.
         *
         * We can't go back and erase some g-code very easily.
         */
        bool isSequential() const override;

        /*
         * \brief Test if there are any more slices to be made.
         */
        bool hasSlice() const override;

        /*
         * \brief Send the current position.
         *
         * The command line doesn't do anything with the current position so this is
         * ignored.
         */
        void sendCurrentPosition(const Point&) override;

        /*
         * \brief Indicate to the command line that we finished slicing.
         *
         * The command line doesn't do anything with that information so this is
         * ignored.
         */
        void sendFinishedSlicing() const override;

        /*
         * \brief Output the g-code header.
         */
        void sendGCodePrefix(const std::string&) const override;

        /*
         * \brief Send the uuid of the generated slice so that it may be processed by
         * the front-end.
         */
        void sendSliceUUID(const std::string& slice_uuid) const override;

        /*
         * \brief Indicate that the layer has been completely sent.
         *
         * The command line doesn't do anything with that information so this is
         * ignored.
         */
        void sendLayerComplete(const LayerIndex&, const coord_t&, const coord_t&) override;

        /*
         * \brief Send a line for display.
         *
         * The command line doesn't show any layer view so this is ignored.
         */
        void sendLineTo(const PrintFeatureType&, const Point&, const coord_t&, const coord_t&, const Velocity&) override;

        /*
         * \brief Complete a layer to show it in layer view.
         *
         * The command line doesn't show any layer view so this is ignored.
         */
        void sendOptimizedLayerData() override;

        /*
         * \brief Send a polygon to show it in layer view.
         *
         * The command line doesn't show any layer view so this is ignored.
         */
        void sendPolygon(const PrintFeatureType&, const ConstPolygonRef&, const coord_t&, const coord_t&, const Velocity&) override;

        /*
         * \brief Send a polygon to show it in layer view.
         *
         * The command line doesn't show any layer view so this is ignored.
         */
        void sendPolygons(const PrintFeatureType&, const Polygons&, const coord_t&, const coord_t&, const Velocity&) override;

        /*
         * \brief Show an estimate of how long the print would take and how much
         * material it would use.
         */
        void sendPrintTimeMaterialEstimates() const override;

        /*
         * \brief Show an update of our slicing progress.
         */
        void sendProgress(const float& progress) const override;

        /*
         * \brief Set which extruder is being used for the following calls to
         * ``sendPolygon``, ``sendPolygons`` and ``sendLineTo``.
         *
         * This has no effect though because we don't show these three functions
         * because the command line doesn't show layer view.
         */
        void setExtruderForSend(const ExtruderTrain&) override;

        /*
         * \brief Set which layer is being used for the following calls to
         * ``sendPolygon``, ``sendPolygons`` and ``sendLineTo``.
         *
         * This has no effect though because we don't shwo these three functions
         * because the command line doesn't show layer view.
         */
        void setLayerForSend(const LayerIndex&) override;

        /*
         * \brief Slice the next scene that the command line commands us to slice.
         */
        void sliceNext() override;
    private:
        /*
         * \brief The command line arguments that the application was called with.
         */
        std::vector<std::string> arguments;
        Slice* current_slice = nullptr;
    public:
        ccglobal::Tracer* m_tracer;
    };

} //namespace cura52

#endif //COMMANDLINE_H