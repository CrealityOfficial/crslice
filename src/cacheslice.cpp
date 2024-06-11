#include "crslice2/cacheslice.h"
#include "crslice2/crscene.h"
#include "../wrapper/orcaslicewrapper.h"
#include "conv.h"

namespace crslice2
{
	void debug_expolygons(const Slic3r::ExPolygons& polys, ccglobal::VisualDebugger* debugger)
	{
		float out_width = 2.0f;
		float in_width = 1.0f;
		trimesh::vec3 out_color = trimesh::vec3(1.0f, 0.0f, 0.0f);
		trimesh::vec3 in_color = trimesh::vec3(0.0f, 1.0f, 0.0f);
		for (const Slic3r::ExPolygon& poly : polys)
		{
			debugger->visual_polygon(convert(poly.contour), out_color, out_width);
			for (const Slic3r::Polygon& pol : poly.holes)
			{
				debugger->visual_polygon(convert(pol), in_color, in_width);
			}
		}
	}

	class CacheSliceImpl
	{
	public:
		CacheSliceImpl() {
			result.reset();
		}

		~CacheSliceImpl() {

		}

		void slice(CrScenePtr scene, const CacheSliceParam& param, const Slic3r::Model& model, Slic3r::DynamicPrintConfig& config, ccglobal::Tracer* tracer) {
			int alreadyShow = 0;
			Slic3r::PrintBase::status_callback_type callback = [&tracer, &alreadyShow](const Slic3r::PrintBase::SlicingStatus& _status) {
				if (tracer && alreadyShow <= _status.percent)
				{
					alreadyShow = _status.percent;
					tracer->progress((float)_status.percent * 0.01);
					tracer->message(_status.text.c_str());

					if (tracer->interrupt())
					{
						throw Slic3r::SlicingError("User Cancelled", 0);
					}
				}
			};
			print.set_callback(callback);
			
			print.setMultiColor(detect_multi_color_slice(config, model, tracer));
			print.apply(model, config);

			print.is_BBL_printer() = print.getMultiColor();
			print.set_plate_origin(Slic3r::Vec3d(0.0, 0.0, 0.0));
			print.set_plate_index(0);

			print.validate(&warning, &polygons, &height_polygons);

			try {
				print.process();
			}
			catch (const Slic3r::SlicingError& e1)
			{
				return _handle_slice_exception(print, e1.objectId(), e1.what(), tracer);
			}
			catch (const Slic3r::SlicingErrors& e2) {

				return  _handle_slice_exception(print, e2.errors_[0].objectId(), e2.errors_[0].what(), tracer);
			}

			try
			{
				print.export_gcode(param.outName, &result);
			}
			catch (const std::exception& ex)
			{
				//tracer->failed("export gcode failed@");
				std::string error = ex.what();
				error += "@";
				tracer->failed(error.c_str());
				return;
			}
		}

		Slic3r::Print print;

		//result
		Slic3r::GCodeProcessorResult result;
		Slic3r::StringObjectException warning;
		Slic3r::Polygons polygons;
		std::vector<std::pair<Slic3r::Polygon, float>> height_polygons;
	};

	CacheSlice::CacheSlice()
		:m_impl(new CacheSliceImpl())
	{

	}

	CacheSlice::~CacheSlice()
	{
		delete m_impl;
	}

	void CacheSlice::slice(const CacheSliceParam& param, ccglobal::Tracer* tracer)
	{
		CrScenePtr scene(new crslice2::CrScene());
		scene->load(param.fileName);

		Slic3r::Model model;
		Slic3r::DynamicPrintConfig config;

		Slic3r::Calib_Params calibParams;
		calibParams.end = 0.0;
		calibParams.start = 0.0;
		calibParams.step = 0.0;
		calibParams.print_numbers = false;
		Slic3r::ThumbnailsList thumbnailData;

		convert_scene_2_orca(scene, model, config, calibParams, thumbnailData);

		m_impl->slice(scene, param, model, config, tracer);
	}

	void CacheSlice::visual_raw_slices(int layer, ccglobal::VisualDebugger* debugger)
	{
		if (!debugger)
			return;

		Slic3r::PrintObjectPtrs objs = m_impl->print.objects_mutable();
		int size = (int)objs.size();
		for (int i = 0; i < size; ++i)
		{
			Slic3r::PrintObject* obj = objs.at(i);
			Slic3r::Layer* layer = obj->get_layer(i);
			if (!layer)
				continue;

			debug_expolygons(layer->lslices, debugger);
		}
	}

	//cache interface
	class CrSlicePrintImpl
	{
	public:
		CrSlicePrintImpl() {

		}

		~CrSlicePrintImpl() {

		}

		Slic3r::Print print;
	};

	CrSlicePrint::CrSlicePrint()
		:impl(new CrSlicePrintImpl())
	{
	}

	CrSlicePrint::~CrSlicePrint()
	{
		delete impl;
		impl = nullptr;
	}

	class CrSliceModelImpl
	{
	public:
		CrSliceModelImpl() {

		}

		~CrSliceModelImpl() {

		}

		Slic3r::Model model;
		Slic3r::DynamicPrintConfig config;
	};

	CrSliceModel::CrSliceModel()
		:impl(new CrSliceModelImpl())
	{
	}

	CrSliceModel::~CrSliceModel()
	{
		delete impl;
		impl = nullptr;
	}

	CrSliceObject* CrSliceModel::add_object()
	{
		CrSliceObject* object = new CrSliceObject();
		return object;
	}

	void CrSliceModel::setParameter(const std::string& key, const std::string& value)
	{

	}

	class CrSliceObjectImpl
	{
	public:
		CrSliceObjectImpl() {

		}

		~CrSliceObjectImpl() {

		}

	};

	CrSliceObject::CrSliceObject()
		:impl(new CrSliceObjectImpl())
	{
	}

	CrSliceObject::~CrSliceObject()
	{

	}

	void CrSliceObject::setParameter(const std::string& key, const std::string& value)
	{

	}

	void CrSliceObject::setMatrix(const trimesh::xform& matrix)
	{

	}

	CrSliceVolume* CrSliceObject::add_volume()
	{
		return nullptr;
	}

	class CrSliceVolumeImpl
	{
	public:
		CrSliceVolumeImpl() {

		}

		~CrSliceVolumeImpl() {

		}
	};

	CrSliceVolume::CrSliceVolume()
		:impl(new CrSliceVolumeImpl())
	{
	}

	CrSliceVolume::~CrSliceVolume()
	{
	};

	void CrSliceVolume::setParameter(const std::string& key, const std::string& value)
	{

	}

	void CrSliceVolume::setMatrix(const trimesh::xform& matrix)
	{

	}

	void CrSliceVolume::setMeshData(TriMeshPtr mesh)
	{

	}

	void CrSliceVolume::setSpreadColor(const std::vector<std::string>& colors)
	{

	}

	void CrSliceVolume::setSpreadSeam(const std::vector<std::string>& seams)
	{

	}

	void CrSliceVolume::setSpreadSupport(const std::vector<std::string>& supports)
	{

	}

	void CrSliceVolume::setName(const std::string& name)
	{

	}

	void CrSliceVolume::setLayerHeight(const std::vector<double>& layer_heights)
	{

	}

	void CrSliceVolume::setModelType(const int model_type)
	{

	}

	CrSliceResult slice(CrSlicePrint& print, CrSliceModel& model, const std::string& out_file, ccglobal::Tracer* tracer)
	{
		CrSliceResult result;

		OrcaResult orca_result;
		Slic3r::Calib_Params calib_params;
		auto f = [&](const Slic3r::ThumbnailsParams&) { return Slic3r::ThumbnailsList(); };
		Slic3r::ThumbnailsGeneratorCallback thumbnail_callback = f;
		orca_slice_impl(print.impl->print, model.impl->model, model.impl->config, out_file, thumbnail_callback, calib_params, orca_result, tracer);

		return result;
	}
}
