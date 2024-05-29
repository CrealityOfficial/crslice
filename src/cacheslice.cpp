#include "crslice2/cacheslice.h"
#include "crslice2/crscene.h"
#include "../wrapper/orcaslicewrapper.h"

namespace crslice2
{
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
}
