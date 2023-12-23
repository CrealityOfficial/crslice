
set(CMAKE_CXX_STANDARD 17)

if (MINGW)
    add_compile_options(-Wa,-mbig-obj)
endif ()

__cc_find(boost_static)
__cc_find(eigen)
__cc_find(cereal)
__cc_find(admesh)
__cc_find(tbb_static)
__cc_find(clipper2)

__assert_target(clipper2)
__assert_target(cereal)
__assert_target(eigen)
__assert_target(admesh)
__assert_target(tbb_static)

set(items
	#libslic3r_version.h
	Slice3rBase/MultiPoint.cpp
    Slice3rBase/MultiPoint.hpp
    Slice3rBase/ArcFitter.cpp
    Slice3rBase/ArcFitter.hpp
    #pchheader.cpp
    #pchheader.hpp
    Slice3rBase/AABBTreeIndirect.hpp
    Slice3rBase/AABBTreeLines.hpp
    Slice3rBase/BoundingBox.cpp
    Slice3rBase/BoundingBox.hpp
    #BridgeDetector.cpp
    #BridgeDetector.hpp
    #FaceDetector.cpp
    #FaceDetector.hpp
    #Brim.cpp
    #Brim.hpp
    #BuildVolume.cpp
    #BuildVolume.hpp
    Slice3rBase/Circle.cpp
    Slice3rBase/Circle.hpp
    Slice3rBase/clipper.cpp
    Slice3rBase/clipper.hpp
    Slice3rBase/ClipperUtils.cpp
    Slice3rBase/ClipperUtils.hpp
    Slice3rBase/Clipper2Utils.cpp
    Slice3rBase/Clipper2Utils.hpp
    #Config.cpp
    #Config.hpp
    #CurveAnalyzer.cpp
    #CurveAnalyzer.hpp
    #EdgeGrid.cpp
    #EdgeGrid.hpp
    #ElephantFootCompensation.cpp
    #ElephantFootCompensation.hpp
    #enum_bitmask.hpp
    Slice3rBase/ExPolygon.cpp
    Slice3rBase/ExPolygon.hpp
    #Extruder.cpp
    #Extruder.hpp
    #ExtrusionEntity.cpp
    #ExtrusionEntity.hpp
    #ExtrusionEntityCollection.cpp
    #ExtrusionEntityCollection.hpp
    #ExtrusionSimulator.cpp
    #ExtrusionSimulator.hpp
    #FileParserError.hpp
    #Fill/Fill.cpp
    #Fill/Fill.hpp
    #Fill/Fill3DHoneycomb.cpp
    #Fill/Fill3DHoneycomb.hpp
    #Fill/FillAdaptive.cpp
    #Fill/FillAdaptive.hpp
    #Fill/FillBase.cpp
    #Fill/FillBase.hpp
    #Fill/FillConcentric.cpp
    #Fill/FillConcentric.hpp
    #Fill/FillConcentricInternal.cpp
    #Fill/FillConcentricInternal.hpp
    #Fill/FillHoneycomb.cpp
    #Fill/FillHoneycomb.hpp
    #Fill/FillGyroid.cpp
    #Fill/FillGyroid.hpp
    #Fill/FillPlanePath.cpp
    #Fill/FillPlanePath.hpp
    #Fill/FillLine.cpp
    #Fill/FillLine.hpp
    #Fill/FillLightning.cpp
    #Fill/FillLightning.hpp
    #Fill/Lightning/DistanceField.cpp
    #Fill/Lightning/DistanceField.hpp
    #Fill/Lightning/Generator.cpp
    #Fill/Lightning/Generator.hpp
    #Fill/Lightning/Layer.cpp
    #Fill/Lightning/Layer.hpp
    #Fill/Lightning/TreeNode.cpp
    #Fill/Lightning/TreeNode.hpp
    #Fill/FillRectilinear.cpp
    #Fill/FillRectilinear.hpp
    #Flow.cpp
    #Flow.hpp
    #format.hpp
	#Format/3mf.cpp
	#Format/3mf.hpp
	#Format/bbs_3mf.cpp
	#Format/bbs_3mf.hpp
    #Format/AMF.cpp
    #Format/AMF.hpp
    #Format/OBJ.cpp
    #Format/OBJ.hpp
    #Format/objparser.cpp
    #Format/objparser.hpp
    #Format/STEP.cpp
    #Format/STEP.hpp
    #Format/STL.cpp
    #Format/STL.hpp
	#Format/SL1.hpp
	#Format/SL1.cpp
	#Format/svg.hpp
    #Format/svg.cpp
	#GCode/ThumbnailData.cpp
	#GCode/ThumbnailData.hpp
    #GCode/CoolingBuffer.cpp
    #GCode/CoolingBuffer.hpp
	#GCode/FanMover.cpp
    #GCode/FanMover.hpp
    #GCode/PostProcessor.cpp
    #GCode/PostProcessor.hpp
	##GCode/PressureEqualizer.cpp
	##GCode/PressureEqualizer.hpp
    #GCode/PrintExtents.cpp
    #GCode/PrintExtents.hpp
    #GCode/RetractWhenCrossingPerimeters.cpp
    #GCode/RetractWhenCrossingPerimeters.hpp
    #GCode/SpiralVase.cpp
    #GCode/SpiralVase.hpp
    #GCode/SeamPlacer.cpp
    #GCode/SeamPlacer.hpp
    #GCode/ToolOrdering.cpp
    #GCode/ToolOrdering.hpp
    #GCode/WipeTower.cpp
    #GCode/WipeTower.hpp
    #GCode/GCodeProcessor.cpp
    #GCode/GCodeProcessor.hpp
    #GCode/AvoidCrossingPerimeters.cpp
    #GCode/AvoidCrossingPerimeters.hpp
    #GCode/ExtrusionProcessor.hpp
    #GCode/ConflictChecker.cpp
    #GCode/ConflictChecker.hpp
    #GCode.cpp
    #GCode.hpp
    #GCodeReader.cpp
    #GCodeReader.hpp
    ## GCodeSender.cpp
    ## GCodeSender.hpp
    #GCodeWriter.cpp
    #GCodeWriter.hpp
    Slice3rBase/Geometry.cpp
    Slice3rBase/Geometry.hpp
    Slice3rBase/Geometry/Bicubic.hpp
    Slice3rBase/Geometry/Circle.cpp
    Slice3rBase/Geometry/Circle.hpp
    Slice3rBase/Geometry/ConvexHull.cpp
    Slice3rBase/Geometry/ConvexHull.hpp
    Slice3rBase/Geometry/Curves.hpp
    Slice3rBase/Geometry/MedialAxis.cpp
    Slice3rBase/Geometry/MedialAxis.hpp
    #Geometry/Voronoi.hpp
    #Geometry/VoronoiOffset.cpp
    #Geometry/VoronoiOffset.hpp
    #Geometry/VoronoiUtilsCgal.cpp
    #Geometry/VoronoiUtilsCgal.hpp
    #Geometry/VoronoiVisualUtils.hpp
    Slice3rBase/Int128.hpp
    #InternalBridgeDetector.cpp
    #InternalBridgeDetector.hpp
    #KDTreeIndirect.hpp
    #Layer.cpp
    #Layer.hpp
    #LayerRegion.cpp
    Slice3rBase/libslic3r.h
    Slice3rBase/Line.cpp
    Slice3rBase/Line.hpp
    #BlacklistedLibraryCheck.cpp
    #BlacklistedLibraryCheck.hpp
    Slice3rBase/LocalesUtils.cpp
    Slice3rBase/LocalesUtils.hpp
    #Model.cpp
    #Model.hpp
    #ModelArrange.hpp
    #ModelArrange.cpp
    #MultiMaterialSegmentation.cpp
    #MultiMaterialSegmentation.hpp
    #CustomGCode.cpp
    #CustomGCode.hpp
    #Arrange.hpp
    #Arrange.cpp
    #NormalUtils.cpp
    #NormalUtils.hpp
    #Orient.hpp
    #Orient.cpp
    #MutablePriorityQueue.hpp
    #ObjectID.cpp
    #ObjectID.hpp
    #PerimeterGenerator.cpp
    #PerimeterGenerator.hpp
    #PlaceholderParser.cpp
    #PlaceholderParser.hpp
    #Platform.cpp
    #Platform.hpp
    Slice3rBase/Point.cpp
    Slice3rBase/Point.hpp
    Slice3rBase/Polygon.cpp
    Slice3rBase/Polygon.hpp
    #MutablePolygon.cpp
    #MutablePolygon.hpp
    #PolygonTrimmer.cpp
    #PolygonTrimmer.hpp
    Slice3rBase/Polyline.cpp
    Slice3rBase/Polyline.hpp
    #Preset.cpp
    #Preset.hpp
    #PresetBundle.cpp
    #PresetBundle.hpp
    #ProjectTask.cpp
    #ProjectTask.hpp
    #PrincipalComponents2D.hpp
    #PrincipalComponents2D.cpp
	#AppConfig.cpp
	#AppConfig.hpp
    #Print.cpp
    #Print.hpp
    #PrintApply.cpp
    #PrintBase.cpp
    #PrintBase.hpp
    #PrintConfig.cpp
    #PrintConfig.hpp
    #PrintObject.cpp
    #PrintObjectSlice.cpp
    #PrintRegion.cpp
    #PNGReadWrite.hpp
    #PNGReadWrite.cpp
    #QuadricEdgeCollapse.cpp
    #QuadricEdgeCollapse.hpp
    #Semver.cpp
    #ShortEdgeCollapse.cpp
    #ShortEdgeCollapse.hpp
    #ShortestPath.cpp
    #ShortestPath.hpp
    #SLAPrint.cpp
    #SLAPrintSteps.cpp
    #SLAPrintSteps.hpp
    #SLAPrint.hpp
    #Slicing.cpp
    #Slicing.hpp
    #SlicesToTriangleMesh.hpp
    #SlicesToTriangleMesh.cpp
    #SlicingAdaptive.cpp
    #SlicingAdaptive.hpp
    #SupportMaterial.cpp
    #SupportMaterial.hpp
    #PrincipalComponents2D.cpp
    #PrincipalComponents2D.hpp
    #SupportSpotsGenerator.cpp
    #SupportSpotsGenerator.hpp
    #TreeSupport.hpp
    #TreeSupport.cpp
    #MinimumSpanningTree.hpp
    #MinimumSpanningTree.cpp
    #Surface.cpp
    #Surface.hpp
    #SurfaceCollection.cpp
    #SurfaceCollection.hpp
    #SVG.cpp
    #SVG.hpp
    Slice3rBase/Technologies.hpp
    Slice3rBase/Tesselate.cpp
    Slice3rBase/Tesselate.hpp
    Slice3rBase/TriangleMesh.cpp
    Slice3rBase/TriangleMesh.hpp
    Slice3rBase/TriangleMeshSlicer.cpp
    Slice3rBase/TriangleMeshSlicer.hpp
    #MeshSplitImpl.hpp
    #TriangulateWall.hpp
    #TriangulateWall.cpp
    #utils.cpp
    #Utils.hpp
    #Time.cpp
    #Time.hpp
    #Thread.cpp
    #Thread.hpp
    #TriangleSelector.cpp
    #TriangleSelector.hpp
    #TriangleSetSampling.cpp
    #TriangleSetSampling.hpp
    Slice3rBase/MTUtils.hpp
    #VariableWidth.cpp
    #VariableWidth.hpp
    #Zipper.hpp
    #Zipper.cpp
    Slice3rBase/MinAreaBoundingBox.hpp
    Slice3rBase/MinAreaBoundingBox.cpp
    #miniz_extension.hpp
    #miniz_extension.cpp
    #MarchingSquares.hpp
    Slice3rBase/Execution/Execution.hpp
    Slice3rBase/Execution/ExecutionSeq.hpp
    Slice3rBase/Execution/ExecutionTBB.hpp
    #Optimize/Optimizer.hpp
    #Optimize/NLoptOptimizer.hpp
    #Optimize/BruteforceOptimizer.hpp
    #Slice3rBase/SLA/Pad.hpp
    #Slice3rBase/SLA/Pad.cpp
    #Slice3rBase/SLA/SupportTreeBuilder.hpp
    #Slice3rBase/SLA/SupportTreeMesher.hpp
    #Slice3rBase/SLA/SupportTreeMesher.cpp
    #Slice3rBase/SLA/SupportTreeBuildsteps.hpp
    #Slice3rBase/SLA/SupportTreeBuildsteps.cpp
    #Slice3rBase/SLA/SupportTreeBuilder.cpp
    Slice3rBase/SLA/Concurrency.hpp
    #SLA/SupportTree.hpp
    #SLA/SupportTree.cpp
#    SLA/SupportTreeIGL.cpp
    #SLA/Rotfinder.hpp
    #SLA/Rotfinder.cpp
    Slice3rBase/SLA/BoostAdapter.hpp
    Slice3rBase/SLA/SpatIndex.hpp
    Slice3rBase/SLA/SpatIndex.cpp
    #SLA/RasterBase.hpp
    #SLA/RasterBase.cpp
    #SLA/AGGRaster.hpp
    #SLA/RasterToPolygons.hpp
    #SLA/RasterToPolygons.cpp
    #SLA/ConcaveHull.hpp
    #SLA/ConcaveHull.cpp
    #SLA/Hollowing.hpp
    #SLA/Hollowing.cpp
    Slice3rBase/SLA/JobController.hpp
    Slice3rBase/SLA/SupportPoint.hpp
    Slice3rBase/SLA/SupportPointGenerator.hpp
    Slice3rBase/SLA/SupportPointGenerator.cpp
    Slice3rBase/SLA/IndexedMesh.hpp
    Slice3rBase/SLA/IndexedMesh.cpp
    Slice3rBase/SLA/Clustering.hpp
    Slice3rBase/SLA/Clustering.cpp
    #Slice3rBase/SLA/ReprojectPointsOnMesh.hpp
    #Arachne/BeadingStrategy/BeadingStrategy.hpp
    #Arachne/BeadingStrategy/BeadingStrategy.cpp
    #Arachne/BeadingStrategy/BeadingStrategyFactory.hpp
    #Arachne/BeadingStrategy/BeadingStrategyFactory.cpp
    #Arachne/BeadingStrategy/DistributedBeadingStrategy.hpp
    #Arachne/BeadingStrategy/DistributedBeadingStrategy.cpp
    #Arachne/BeadingStrategy/LimitedBeadingStrategy.hpp
    #Arachne/BeadingStrategy/LimitedBeadingStrategy.cpp
    #Arachne/BeadingStrategy/OuterWallInsetBeadingStrategy.hpp
    #Arachne/BeadingStrategy/OuterWallInsetBeadingStrategy.cpp
    #Arachne/BeadingStrategy/RedistributeBeadingStrategy.hpp
    #Arachne/BeadingStrategy/RedistributeBeadingStrategy.cpp
    #Arachne/BeadingStrategy/WideningBeadingStrategy.hpp
    #Arachne/BeadingStrategy/WideningBeadingStrategy.cpp
    #Arachne/utils/ExtrusionJunction.hpp
    #Arachne/utils/ExtrusionJunction.cpp
    #Arachne/utils/ExtrusionLine.hpp
    #Arachne/utils/ExtrusionLine.cpp
    #Arachne/utils/HalfEdge.hpp
    #Arachne/utils/HalfEdgeGraph.hpp
    #Arachne/utils/HalfEdgeNode.hpp
    #Arachne/utils/SparseGrid.hpp
    #Arachne/utils/SparsePointGrid.hpp
    #Arachne/utils/SparseLineGrid.hpp
    #Arachne/utils/SquareGrid.hpp
    #Arachne/utils/SquareGrid.cpp
    #Arachne/utils/PolygonsPointIndex.hpp
    #Arachne/utils/PolygonsSegmentIndex.hpp
    #Arachne/utils/PolylineStitcher.hpp
    #Arachne/utils/PolylineStitcher.cpp
    #Arachne/utils/VoronoiUtils.hpp
    #Arachne/utils/VoronoiUtils.cpp
    #Arachne/SkeletalTrapezoidation.hpp
    #Arachne/SkeletalTrapezoidation.cpp
    #Arachne/SkeletalTrapezoidationEdge.hpp
    #Arachne/SkeletalTrapezoidationGraph.hpp
    #Arachne/SkeletalTrapezoidationGraph.cpp
    #Arachne/SkeletalTrapezoidationJoint.hpp
    #Arachne/WallToolPaths.hpp
    #Arachne/WallToolPaths.cpp
    #Shape/TextShape.hpp
    #Shape/TextShape.cpp
    #calib.hpp
    #calib.cpp
	Slice3rBase/clipper/clipper_z.cpp
	Slice3rBase/clipper/clipper_z.hpp
	Slice3rBase/fast_float/fast_float.h
    Slice3rBase/overhangquality/extrusionerocessor.hpp
	Slice3rBase/overhangquality/overhanghead.hpp
)

foreach(item ${items})
	list(APPEND SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${item})
endforeach()

if(WIN32)
    list(APPEND LIBS Psapi.lib)
endif()

list(APPEND LIBS eigen clipper2 admesh boost_filesystem boost_nowide cereal)
list(APPEND DEFS BOOST_ALL_NO_LIB TBB_USE_CAPTURED_EXCEPTION=0
	_CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES NOMINMAX)

if(TARGET tbb)
	list(APPEND DEFS SLICE3R_USE_TBB)
	list(APPEND LIBS tbb)
endif()




