#include "crslice2/crslice.h"
#include "wrapper/orcaslicewrapper.h"

#include "ccglobal/log.h"

namespace crslice2
{
	CrSlice::CrSlice()
		:sliceResult({0})
	{

	}

	CrSlice::~CrSlice()
	{

	}

	void CrSlice::sliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer)
	{
		if (!scene)
		{
			LOGM("CrSlice::sliceFromSceneOrca empty scene.");
			return;
		}

		orca_slice_impl(scene, tracer);
	}

	CRSLICE2_API std::vector<double> getLayerHeightProfileAdaptive(crslice2::SettingsPtr settings, trimesh::TriMesh* triMesh, float quality)
	{
		return orca_layer_height_profile_adaptive(settings, triMesh, quality);
	}

	CRSLICE2_API std::vector<double> smooth_height_profile(crslice2::SettingsPtr settings, trimesh::TriMesh* triMesh,
		const std::vector<double>& layer, unsigned int radius, bool keep_min)
	{
		return orca_smooth_height_profile(settings, triMesh, layer, radius, keep_min);
	}

	void orcaSliceFromFile(const std::string& file, const std::string& out)
	{
		orca_slice_fromfile_impl(file, out);
	}
}