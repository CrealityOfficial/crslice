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

		prusa_slice_impl(scene, tracer);
	}

	void prusaSliceFromFile(const std::string& file, const std::string& out)
	{
		prusa_slice_fromfile_impl(file, out);
	}
}