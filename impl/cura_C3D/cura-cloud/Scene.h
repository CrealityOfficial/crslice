//Copyright (c) 2018 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef CLOUD_SCENE_H
#define CLOUD_SCENE_H

#include "cxutil/slicer/ExtruderTrain.h" //To store the extruders in the scene.
#include "cxutil/slicer/MeshGroup.h" //To store the mesh groups in the scene.
#include "cxutil/settings/Settings.h" //To store the global settings.
#include "FffProcessor.h"
#include "sliceDataStorage.h"
#include "progress/Progress.h"
#include "cxutil/slicer/callback.h"
#include "cxutil/slicer/scallback.h"
#include "cxutil/util/macros.h"



namespace cxutil
{
    /*
     * Represents a scene that should be sliced.
     */
    class Communication;
    class Scene
    {
    public:
        cxProgressFunc pFunc;
        cxInterruptFunc iFunc;
        cxFailedFunc fFunc;

        Communication* communication;
        
		cxutil::SliceCallback* callback;
		cxutil::SGCodeCallback* sCallback;

        cxutil::DebugCallback* debugCallback;

        FffProcessor processor;
        Progress progress;
        long long m_sliceLogSortId;
        /*
         * \brief The global settings in the scene.
         */
        Settings settings;

        /*
         * \brief The mesh groups in the scene.
         */
        std::vector<MeshGroup> mesh_groups;

        /*
         * \brief The extruders in the scene.
         */
        std::vector<ExtruderTrain> extruders;

        //热床加热模块
        std::vector<bool> vctHotZone;

        /*
         * \brief The mesh group that is being processed right now.
         *
         * During initialisation this may be nullptr. For the most part, during the
         * slicing process, you can be assured that this will not be null so you can
         * safely dereference it.
         */
        std::vector<MeshGroup>::iterator current_mesh_group;

        int currentExtruder = -1;  //for group init Extruder
        bool machine_center_is_zero = false;
        /*
         * \brief Create an empty scene.
         *
         * This scene will have no models in it, no extruders, no settings, no
         * nothing.
         * \param num_mesh_groups The number of mesh groups to allocate for.
         */
        Scene(const size_t num_mesh_groups);

        /*
         * \brief Gets a string that contains all settings.
         *
         * This string mimics the command line call of CuraEngine. In theory you
         * could call CuraEngine with this output in the command in order to
         * reproduce the output.
         */
        const std::string getAllSettingsString() const;

        /*
         * \brief Generate the 3D printing instructions to print a given mesh group.
         * \param mesh_group The mesh group to slice.
         */
        void processMeshGroup(MeshGroup& mesh_group, std::vector<Polygons>& skirt_brims);
    private:
        /*
         * \brief You are not allowed to copy the scene.
         */
        Scene(const Scene&) = delete;

        /*
         * \brief You are not allowed to copy by assignment either.
         */
        Scene& operator =(const Scene&) = delete;
    };

} //namespace cxutil

#endif //SCENE_H