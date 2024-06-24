#ifndef CRSLICE_CACHE_SLICE_H_2
#define CRSLICE_CACHE_SLICE_H_2
#include "crslice2/header.h"
#include "ccglobal/debugger.h"

namespace crslice2
{
	struct CacheSliceParam
	{
		std::string fileName;
		std::string outName;
	};

	class CacheSliceImpl;
	class CRSLICE2_API CacheSlice
	{
	public:
		CacheSlice();
		~CacheSlice();

		void slice(const CacheSliceParam& param, ccglobal::Tracer* tracer = nullptr);

		void visual_raw_slices(int layer, ccglobal::VisualDebugger* debugger);
	protected:
		CacheSliceImpl* m_impl;
	};

	/////cache interface
	class CrSliceObject;
	class CrSliceVolume;
	class CrSlicePrintImpl;
	class CRSLICE2_API CrSlicePrint
	{
	public:
		CrSlicePrint();
		~CrSlicePrint();

		CrSlicePrintImpl* impl;
	};

	class CrSliceModelImpl;
	class CRSLICE2_API CrSliceModel
	{
	public:
		CrSliceModel();
		~CrSliceModel();

		void setParameter(const std::string& key, const std::string& value);
		CrSliceObject* add_object();
		void remove_object(CrSliceObject* object);

		CrSliceModelImpl* impl;
		std::list<CrSliceObject*> m_objects;
	};

	class CrSliceObjectImpl;
	class CRSLICE2_API CrSliceObject
	{
	public:
		CrSliceObject();
		~CrSliceObject();

		void setName(const std::string& name);
		void setParameter(const std::string& key, const std::string& value);
		void setMatrix(const trimesh::xform& matrix);
		void setLayerHeight(const std::vector<double>& layer_heights);
		void setHostID(int64_t id);

		CrSliceVolume* add_volume();
		void remove_volume(CrSliceVolume* volume);

		CrSliceObjectImpl* impl;
	};

	class CrSliceVolumeImpl;
	class CRSLICE2_API CrSliceVolume
	{
	public:
		CrSliceVolume();
		~CrSliceVolume();

		void setParameter(const std::string& key, const std::string& value);
		void setMatrix(const trimesh::xform& matrix);
		void setMeshData(TriMeshPtr mesh);
		void setSpreadColor(const std::vector<std::string>& colors);
		void setSpreadSeam(const std::vector<std::string>& seams);
		void setSpreadSupport(const std::vector<std::string>& supports);
		void setName(const std::string& name);
		void setModelType(const int model_type);
		void setHostID(int64_t id);

		CrSliceVolumeImpl* impl;
	};

	struct CrSliceResult
	{
		bool success = true;
		std::map<std::string, std::pair<std::string, int64_t>> warnings;
	};

	CRSLICE2_API void update_print_volume_state(CrSliceModel& model, const std::vector<trimesh::dvec2>& shapes, double height);

	CRSLICE2_API CrSliceResult slice(CrSlicePrint& print, CrSliceModel& model, const std::vector<ThumbnailData>& thumbnails,
		const std::string& out_file, ccglobal::Tracer* tracer = nullptr);
}
#endif  // MSIMPLIFY_SIMPLIFY_H
