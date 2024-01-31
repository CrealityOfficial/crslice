cmake_minimum_required(VERSION 3.13)
project(libslic3r)

set(CMAKE_CXX_STANDARD 17)
string(TIMESTAMP COMPILE_TIME %Y%m%d-%H%M%S)
set(SLIC3R_BUILD_TIME ${COMPILE_TIME})

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libslic3r_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/libslic3r_version.h @ONLY)

__enable_bigobj()

set(OpenVDBUtils_SOURCES "")
if (TARGET OpenVDB::openvdb)
    set(OpenVDBUtils_SOURCES OpenVDBUtils.cpp OpenVDBUtils.hpp)
endif()

#find_package(LibBGCode REQUIRED COMPONENTS Convert)
#slic3r_remap_configs(LibBGCode::bgcode_core RelWithDebInfo Release)
#slic3r_remap_configs(LibBGCode::bgcode_binarize RelWithDebInfo Release)
#slic3r_remap_configs(LibBGCode::bgcode_convert RelWithDebInfo Release)

set(SLIC3R_SOURCES
    #pchheader.cpp
    #pchheader.hpp
    AStar.hpp
    AABBTreeIndirect.hpp
    AABBTreeLines.hpp
    AABBMesh.hpp
    AABBMesh.cpp
    Algorithm/PathSorting.hpp
    Algorithm/RegionExpansion.hpp
    Algorithm/RegionExpansion.cpp
    AnyPtr.hpp
    BoundingBox.cpp
    BoundingBox.hpp
    BridgeDetector.cpp
    BridgeDetector.hpp
    Brim.cpp
    Brim.hpp
    BuildVolume.cpp
    BuildVolume.hpp
    BoostAdapter.hpp
    clipper.cpp
    clipper.hpp
    ClipperUtils.cpp
    ClipperUtils.hpp
    ClipperZUtils.hpp
    Color.cpp
    Color.hpp
    Config.cpp
    Config.hpp
    CSGMesh/CSGMesh.hpp
    CSGMesh/SliceCSGMesh.hpp
    CSGMesh/ModelToCSGMesh.hpp
    CSGMesh/PerformCSGMeshBooleans.hpp
    CSGMesh/VoxelizeCSGMesh.hpp
    CSGMesh/TriangleMeshAdapter.hpp
    CSGMesh/CSGMeshCopy.hpp
    EdgeGrid.cpp
    EdgeGrid.hpp
    ElephantFootCompensation.cpp
    ElephantFootCompensation.hpp
    Emboss.cpp
    Emboss.hpp
    EmbossShape.hpp
    enum_bitmask.hpp
    ExPolygon.cpp
    ExPolygon.hpp
    ExPolygonSerialize.hpp
    ExPolygonsIndex.cpp
    ExPolygonsIndex.hpp
    Extruder.cpp
    Extruder.hpp
    ExtrusionEntity.cpp
    ExtrusionEntity.hpp
    ExtrusionEntityCollection.cpp
    ExtrusionEntityCollection.hpp
    ExtrusionRole.cpp
    ExtrusionRole.hpp
    ExtrusionSimulator.cpp
    ExtrusionSimulator.hpp
    FileParserError.hpp
    Fill/Fill.cpp
    Fill/Fill.hpp
    Fill/Fill3DHoneycomb.cpp
    Fill/Fill3DHoneycomb.hpp
    Fill/FillAdaptive.cpp
    Fill/FillAdaptive.hpp
    Fill/FillBase.cpp
    Fill/FillBase.hpp
    Fill/FillConcentric.cpp
    Fill/FillConcentric.hpp
    Fill/FillEnsuring.cpp
    Fill/FillEnsuring.hpp
    Fill/FillHoneycomb.cpp
    Fill/FillHoneycomb.hpp
    Fill/FillGyroid.cpp
    Fill/FillGyroid.hpp
    Fill/FillPlanePath.cpp
    Fill/FillPlanePath.hpp
    Fill/FillLine.cpp
    Fill/FillLine.hpp
    Fill/FillLightning.cpp
    Fill/FillLightning.hpp
    Fill/Lightning/DistanceField.cpp
    Fill/Lightning/DistanceField.hpp
    Fill/Lightning/Generator.cpp
    Fill/Lightning/Generator.hpp
    Fill/Lightning/Layer.cpp
    Fill/Lightning/Layer.hpp
    Fill/Lightning/TreeNode.cpp
    Fill/Lightning/TreeNode.hpp
    Fill/FillRectilinear.cpp
    Fill/FillRectilinear.hpp
    Flow.cpp
    Flow.hpp
    format.hpp
    Format/3mf.cpp
    Format/3mf.hpp
    Format/AMF.cpp
    Format/AMF.hpp
    Format/OBJ.cpp
    Format/OBJ.hpp
    Format/objparser.cpp
    Format/objparser.hpp
    Format/STL.cpp
    Format/STL.hpp
    Format/SLAArchiveWriter.hpp
    Format/SLAArchiveWriter.cpp
    Format/SLAArchiveReader.hpp
    Format/SLAArchiveReader.cpp
    Format/ZipperArchiveImport.hpp
    Format/ZipperArchiveImport.cpp
    Format/SL1.hpp
    Format/SL1.cpp
    Format/SL1_SVG.hpp
    Format/SL1_SVG.cpp
    Format/AnycubicSLA.hpp
    Format/AnycubicSLA.cpp
    Format/STEP.hpp
    Format/STEP.cpp
    Format/SVG.hpp
    Format/SVG.cpp
    Format/SLAArchiveFormatRegistry.hpp
    Format/SLAArchiveFormatRegistry.cpp
    GCode/ThumbnailData.cpp
    GCode/ThumbnailData.hpp
    GCode/Thumbnails.cpp
    GCode/Thumbnails.hpp
    GCode/ConflictChecker.cpp
    GCode/ConflictChecker.hpp
    GCode/CoolingBuffer.cpp
    GCode/CoolingBuffer.hpp
    GCode/ExtrusionProcessor.cpp
    GCode/ExtrusionProcessor.hpp
    GCode/FindReplace.cpp
    GCode/FindReplace.hpp
    GCode/LabelObjects.cpp
    GCode/LabelObjects.hpp
    GCode/GCodeWriter.cpp
    GCode/GCodeWriter.hpp
    GCode/PostProcessor.cpp
    GCode/PostProcessor.hpp
    GCode/PressureEqualizer.cpp
    GCode/PressureEqualizer.hpp
    GCode/PrintExtents.cpp
    GCode/PrintExtents.hpp
    GCode/RetractWhenCrossingPerimeters.cpp
    GCode/RetractWhenCrossingPerimeters.hpp
    GCode/SpiralVase.cpp
    GCode/SpiralVase.hpp
    GCode/SeamPlacer.cpp
    GCode/SeamPlacer.hpp
    GCode/SmoothPath.cpp
    GCode/SmoothPath.hpp
    GCode/ToolOrdering.cpp
    GCode/ToolOrdering.hpp
    GCode/Wipe.cpp
    GCode/Wipe.hpp
    GCode/WipeTower.cpp
    GCode/WipeTower.hpp
    GCode/WipeTowerIntegration.cpp
    GCode/WipeTowerIntegration.hpp
    GCode/GCodeProcessor.cpp
    GCode/GCodeProcessor.hpp
    GCode/AvoidCrossingPerimeters.cpp
    GCode/AvoidCrossingPerimeters.hpp
    GCode.cpp
    GCode.hpp
    GCodeReader.cpp
    GCodeReader.hpp
    # GCodeSender.cpp
    # GCodeSender.hpp
    Geometry.cpp
    Geometry.hpp
    Geometry/ArcWelder.cpp
    Geometry/ArcWelder.hpp
    Geometry/Bicubic.hpp
    Geometry/Circle.cpp
    Geometry/Circle.hpp
    Geometry/ConvexHull.cpp
    Geometry/ConvexHull.hpp
    Geometry/Curves.hpp
    Geometry/MedialAxis.cpp
    Geometry/MedialAxis.hpp
    Geometry/Voronoi.hpp
    Geometry/VoronoiOffset.cpp
    Geometry/VoronoiOffset.hpp
    Geometry/VoronoiVisualUtils.hpp
    Int128.hpp
    JumpPointSearch.cpp
    JumpPointSearch.hpp
    KDTreeIndirect.hpp
    Layer.cpp
    Layer.hpp
    LayerRegion.cpp
    libslic3r.h
    "${CMAKE_CURRENT_BINARY_DIR}/libslic3r_version.h"
    Line.cpp
    Line.hpp
    BlacklistedLibraryCheck.cpp
    BlacklistedLibraryCheck.hpp
    LocalesUtils.cpp
    LocalesUtils.hpp
    CutUtils.cpp
    CutUtils.hpp
    Model.cpp
    Model.hpp
    ModelArrange.hpp
    ModelArrange.cpp
    MultiMaterialSegmentation.cpp
    MultiMaterialSegmentation.hpp
    MeshNormals.hpp
    MeshNormals.cpp
    Measure.hpp
    Measure.cpp
    MeasureUtils.hpp
    CustomGCode.cpp
    CustomGCode.hpp
    Arrange/Arrange.hpp
    Arrange/ArrangeImpl.hpp
    Arrange/Items/ArrangeItem.hpp
    Arrange/Items/ArrangeItem.cpp
    Arrange/Items/SimpleArrangeItem.hpp
    Arrange/Items/SimpleArrangeItem.cpp
    Arrange/Items/TrafoOnlyArrangeItem.hpp
    Arrange/Items/MutableItemTraits.hpp
    Arrange/Items/ArbitraryDataStore.hpp
    Arrange/ArrangeSettingsView.hpp
    Arrange/ArrangeSettingsDb_AppCfg.hpp
    Arrange/ArrangeSettingsDb_AppCfg.cpp
    Arrange/Scene.hpp
    Arrange/Scene.cpp
    Arrange/SceneBuilder.hpp
    Arrange/SceneBuilder.cpp
    Arrange/Tasks/ArrangeTask.hpp
    Arrange/Tasks/ArrangeTaskImpl.hpp
    Arrange/Tasks/FillBedTask.hpp
    Arrange/Tasks/FillBedTaskImpl.hpp
    Arrange/Tasks/MultiplySelectionTask.hpp
    Arrange/Tasks/MultiplySelectionTaskImpl.hpp
    Arrange/SegmentedRectangleBed.hpp
    Arrange/Core/ArrangeItemTraits.hpp
    Arrange/Core/DataStoreTraits.hpp
    Arrange/Core/ArrangeBase.hpp
    Arrange/Core/PackingContext.hpp
    Arrange/Core/ArrangeFirstFit.hpp
    Arrange/Core/Beds.hpp
    Arrange/Core/Beds.cpp
    Arrange/Core/NFP/NFP.hpp
    Arrange/Core/NFP/NFP.cpp
    Arrange/Core/NFP/NFPConcave_CGAL.hpp
    Arrange/Core/NFP/NFPConcave_CGAL.cpp
    Arrange/Core/NFP/NFPConcave_Tesselate.hpp
    Arrange/Core/NFP/NFPConcave_Tesselate.cpp
    Arrange/Core/NFP/EdgeCache.hpp
    Arrange/Core/NFP/EdgeCache.cpp
    Arrange/Core/NFP/CircularEdgeIterator.hpp
    Arrange/Core/NFP/NFPArrangeItemTraits.hpp
    Arrange/Core/NFP/PackStrategyNFP.hpp
    Arrange/Core/NFP/RectangleOverfitPackingStrategy.hpp
    Arrange/Core/NFP/Kernels/KernelTraits.hpp
    Arrange/Core/NFP/Kernels/GravityKernel.hpp
    Arrange/Core/NFP/Kernels/TMArrangeKernel.hpp
    Arrange/Core/NFP/Kernels/CompactifyKernel.hpp
    Arrange/Core/NFP/Kernels/RectangleOverfitKernelWrapper.hpp
    Arrange/Core/NFP/Kernels/SVGDebugOutputKernelWrapper.hpp
    Arrange/Core/NFP/Kernels/KernelUtils.hpp
    MultiPoint.cpp
    MultiPoint.hpp
    MutablePriorityQueue.hpp
    NormalUtils.cpp
    NormalUtils.hpp
    NSVGUtils.cpp
    NSVGUtils.hpp
    ObjectID.cpp
    ObjectID.hpp
    PerimeterGenerator.cpp
    PerimeterGenerator.hpp
    PlaceholderParser.cpp
    PlaceholderParser.hpp
    Platform.cpp
    Platform.hpp
    Point.cpp
    Point.hpp
    Polygon.cpp
    Polygon.hpp
    MutablePolygon.cpp
    MutablePolygon.hpp
    PolygonTrimmer.cpp
    PolygonTrimmer.hpp
    Polyline.cpp
    Polyline.hpp
    Preset.cpp
    Preset.hpp
    PresetBundle.cpp
    PresetBundle.hpp
    PrincipalComponents2D.hpp
    PrincipalComponents2D.cpp
    AppConfig.cpp
    AppConfig.hpp
    Print.cpp
    Print.hpp
    PrintApply.cpp
    PrintBase.cpp
    PrintBase.hpp
    PrintConfig.cpp
    PrintConfig.hpp
    PrintObject.cpp
    PrintObjectSlice.cpp
    PrintRegion.cpp
    PointGrid.hpp
    PNGReadWrite.hpp
    PNGReadWrite.cpp
    QuadricEdgeCollapse.cpp
    QuadricEdgeCollapse.hpp
    Semver.cpp
    ShortEdgeCollapse.cpp
    ShortEdgeCollapse.hpp
    ShortestPath.cpp
    ShortestPath.hpp
    SLAPrint.cpp
    SLAPrintSteps.cpp
    SLAPrintSteps.hpp
    SLAPrint.hpp
    Slicing.cpp
    Slicing.hpp
    SlicesToTriangleMesh.hpp
    SlicesToTriangleMesh.cpp
    SlicingAdaptive.cpp
    SlicingAdaptive.hpp
    Subdivide.cpp
    Subdivide.hpp
    Support/SupportCommon.cpp
    Support/SupportCommon.hpp
    Support/SupportDebug.cpp
    Support/SupportDebug.hpp
    Support/SupportLayer.hpp
    # Support/SupportMaterial.cpp
    # Support/SupportMaterial.hpp
    Support/SupportParameters.cpp
    Support/SupportParameters.hpp
    Support/OrganicSupport.cpp
    Support/OrganicSupport.hpp
    Support/TreeSupport.cpp
    Support/TreeSupport.hpp
    Support/TreeSupportCommon.cpp
    Support/TreeSupportCommon.hpp
    Support/TreeModelVolumes.cpp
    Support/TreeModelVolumes.hpp
    SupportSpotsGenerator.cpp
    SupportSpotsGenerator.hpp
    Surface.cpp
    Surface.hpp
    SurfaceCollection.cpp
    SurfaceCollection.hpp
    SurfaceMesh.hpp
    SVG.cpp
    SVG.hpp
    Technologies.hpp
    Tesselate.cpp
    Tesselate.hpp
    TextConfiguration.hpp
    TriangleMesh.cpp
    TriangleMesh.hpp
    TriangleMeshSlicer.cpp
    TriangleMeshSlicer.hpp
    MeshSplitImpl.hpp
    TriangulateWall.hpp
    utils.cpp
    Utils.hpp
    Time.cpp
    Time.hpp
    Timer.cpp
    Timer.hpp
    Thread.cpp
    Thread.hpp
    TriangleSelector.cpp
    TriangleSelector.hpp
    TriangleSetSampling.cpp
    TriangleSetSampling.hpp
    TriangleSelectorWrapper.cpp
    TriangleSelectorWrapper.hpp
    MTUtils.hpp
    Zipper.hpp
    Zipper.cpp
    MinAreaBoundingBox.hpp
    MinAreaBoundingBox.cpp
    miniz_extension.hpp
    miniz_extension.cpp
    MarchingSquares.hpp
    Execution/Execution.hpp
    Execution/ExecutionSeq.hpp
    Execution/ExecutionTBB.hpp
    Optimize/Optimizer.hpp
    Optimize/NLoptOptimizer.hpp
    Optimize/BruteforceOptimizer.hpp
    #${OpenVDBUtils_SOURCES}
    SLA/Pad.hpp
    SLA/Pad.cpp
    SLA/SupportTreeBuilder.hpp
    SLA/SupportTreeMesher.hpp
    SLA/SupportTreeMesher.cpp
    SLA/SupportTreeUtils.hpp
    SLA/SupportTreeUtilsLegacy.hpp
    SLA/SupportTreeBuilder.cpp
    SLA/SupportTree.hpp
    SLA/SupportTree.cpp
    SLA/SupportTreeStrategies.hpp
#    SLA/SupportTreeIGL.cpp
    SLA/Rotfinder.hpp
    SLA/Rotfinder.cpp
    SLA/SpatIndex.hpp
    SLA/SpatIndex.cpp
    SLA/RasterBase.hpp
    SLA/RasterBase.cpp
    SLA/AGGRaster.hpp
    SLA/RasterToPolygons.hpp
    SLA/RasterToPolygons.cpp
    SLA/ConcaveHull.hpp
    SLA/ConcaveHull.cpp
    SLA/Hollowing.hpp
    SLA/Hollowing.cpp
    SLA/JobController.hpp
    SLA/SupportPoint.hpp
    SLA/SupportPointGenerator.hpp
    SLA/SupportPointGenerator.cpp
    SLA/Clustering.hpp
    SLA/Clustering.cpp
    SLA/ReprojectPointsOnMesh.hpp
    SLA/DefaultSupportTree.hpp
    SLA/DefaultSupportTree.cpp
    SLA/BranchingTreeSLA.hpp
    SLA/BranchingTreeSLA.cpp
    BranchingTree/BranchingTree.cpp
    BranchingTree/BranchingTree.hpp
    BranchingTree/PointCloud.cpp
    BranchingTree/PointCloud.hpp

    Arachne/BeadingStrategy/BeadingStrategy.hpp
    Arachne/BeadingStrategy/BeadingStrategy.cpp
    Arachne/BeadingStrategy/BeadingStrategyFactory.hpp
    Arachne/BeadingStrategy/BeadingStrategyFactory.cpp
    Arachne/BeadingStrategy/DistributedBeadingStrategy.hpp
    Arachne/BeadingStrategy/DistributedBeadingStrategy.cpp
    Arachne/BeadingStrategy/LimitedBeadingStrategy.hpp
    Arachne/BeadingStrategy/LimitedBeadingStrategy.cpp
    Arachne/BeadingStrategy/OuterWallInsetBeadingStrategy.hpp
    Arachne/BeadingStrategy/OuterWallInsetBeadingStrategy.cpp
    Arachne/BeadingStrategy/RedistributeBeadingStrategy.hpp
    Arachne/BeadingStrategy/RedistributeBeadingStrategy.cpp
    Arachne/BeadingStrategy/WideningBeadingStrategy.hpp
    Arachne/BeadingStrategy/WideningBeadingStrategy.cpp
    Arachne/utils/ExtrusionJunction.hpp
    Arachne/utils/ExtrusionJunction.cpp
    Arachne/utils/ExtrusionLine.hpp
    Arachne/utils/ExtrusionLine.cpp
    Arachne/utils/HalfEdge.hpp
    Arachne/utils/HalfEdgeGraph.hpp
    Arachne/utils/HalfEdgeNode.hpp
    Arachne/utils/SparseGrid.hpp
    Arachne/utils/SparsePointGrid.hpp
    Arachne/utils/SparseLineGrid.hpp
    Arachne/utils/SquareGrid.hpp
    Arachne/utils/SquareGrid.cpp
    Arachne/utils/PolygonsPointIndex.hpp
    Arachne/utils/PolygonsSegmentIndex.hpp
    Arachne/utils/PolylineStitcher.hpp
    Arachne/utils/PolylineStitcher.cpp
    Arachne/utils/VoronoiUtils.hpp
    Arachne/utils/VoronoiUtils.cpp
    Arachne/SkeletalTrapezoidation.hpp
    Arachne/SkeletalTrapezoidation.cpp
    Arachne/SkeletalTrapezoidationEdge.hpp
    Arachne/SkeletalTrapezoidationGraph.hpp
    Arachne/SkeletalTrapezoidationGraph.cpp
    Arachne/SkeletalTrapezoidationJoint.hpp
    Arachne/WallToolPaths.hpp
    Arachne/WallToolPaths.cpp
    StaticMap.hpp
	
	clipper/clipper_z.hpp
	clipper/clipper_z.cpp
	admesh/connect.cpp
    admesh/normals.cpp
    admesh/shared.cpp
    admesh/stl.h
    admesh/stl_io.cpp
    admesh/stlinit.cpp
    admesh/util.cpp
	miniz/miniz.h
	miniz/miniz.c
	qoi/qoilib.c
)

list(APPEND lisbslic3r_sources 
	"${CMAKE_CURRENT_SOURCE_DIR}/libslic3r_version.h"
	)
if (APPLE)
    list(APPEND lisbslic3r_sources
			MacUtils.mm
            Format/ModelIO.hpp
            Format/ModelIO.mm
        )
endif ()

__cc_find(boost_static)
__cc_find(eigen)
__cc_find(admesh)
__cc_find(clipper2)
__cc_find(png_static)
__cc_find(openssl)
__cc_find(opencascade_static)
__cc_find(nlopt_static)
__cc_find(expat)
__cc_find(cgal)

__assert_target(clipper2)
__assert_target(eigen)
__assert_target(admesh)
__assert_target(nlopt_static)
__assert_target(expat)

set(libslic3r_dir ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(ILIBS)
set(INTERFACES ${libslic3r_dir} ${CMAKE_CURRENT_SOURCE_DIR}
	#${CMAKE_CURRENT_BINARY_DIR} 
	)
set(INCS ${INTERFACES})
set(DEFS)

if(WIN32)
    list(APPEND ILIBS Psapi.lib)
endif()

list(APPEND ILIBS eigen clipper2 boost_filesystem boost_nowide boost_thread
			 boost_regex png_static ssl crypto expat nlopt_static qhullstatic glu-libtess cgal semver)
list(APPEND DEFS BOOST_ALL_NO_LIB TBB_USE_CAPTURED_EXCEPTION=0
	_CRT_SECURE_NO_WARNINGS _USE_MATH_DEFINES NOMINMAX UNICODE)

if(TARGET tbb_inner)
	list(APPEND DEFS USE_TBB)
	list(APPEND ILIBS tbb_inner tbbmalloc)
endif()

set(OLIBS TKernel
		 TKMath
		 TKG2d
		 TKG3d
		 TKGeomBase
		 TKBRep
		 TKGeomAlgo
		 TKTopAlgo
		 TKPrim
		 TKBO
		 TKShHealing
		 TKBool
		 TKHLR
		 TKMesh
		 TKService
		 TKV3d
		 TKCDF
		 TKLCAF
		 TKCAF
		 TKBinL
		 TKXmlL
		 TKBin
		 TKXml
		 TKVCAF
		 TKXSBase
		 TKSTEPBase
		 TKSTEPAttr
		 TKSTEP209
		 TKSTEP
		 TKIGES
		 TKXCAF
		 TKXDE
		 TKXDEIGES
		 TKXDESTEP
		 TKSTL
		 TKVRML
		 TKRWMesh
		 TKXmlXCAF
		 TKBinXCAF     
		)
list(REVERSE OLIBS)  # for linux build
list(APPEND ILIBS ${OLIBS})

__add_real_target(libslic3r lib 
	SOURCE ${SLIC3R_SOURCES} ${OpenVDBUtils_SOURCES}
	ILIB ${ILIBS}
	INTERFACE ${INTERFACES}
	INC ${INCS}
	DEF ${DEFS}
	FOLDER slice
	)
	
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${lisbslic3r_sources})
#encoding_check(libslic3r)


