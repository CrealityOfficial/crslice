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

	CRSLICE2_API std::vector<double> getLayerHeightProfileAdaptive(SliceParams& slicing_params, trimesh::TriMesh* triMesh, float quality)
	{
		return orca_layer_height_profile_adaptive(slicing_params, triMesh, quality);
	}

	void orcaSliceFromFile(const std::string& file, const std::string& out)
	{
		orca_slice_fromfile_impl(file, out);
	}
}