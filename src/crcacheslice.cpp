#include "crslice/crcacheslice.h"
#include "Application.h"

#include "conv.h"

#include "ccglobal/log.h"
#include "cachedebugger.h"

namespace crslice
{
	CacheLayer::CacheLayer(int _layer, CacheDebugger* _debugger)
		: debugger(_debugger)
		, layer(_layer)
	{

	}

	CacheLayer::~CacheLayer()
	{

	}

	int CacheLayer::partCount()
	{
		return (int)debugger->groupData.partLayers.at(layer).parts.size();
	}

	void CacheLayer::traitAllParts(std::vector<trimesh::vec3>& lines)
	{
		std::vector<cura52::PolygonsPart> parts = debugger->groupData.partLayers.at(layer).parts;
		for (cura52::PolygonsPart& part : parts)
			convert(part.paths, lines);
	}

	CrSCacheSlice::CrSCacheSlice()
	{

	}

	CrSCacheSlice::~CrSCacheSlice()
	{

	}

	void CrSCacheSlice::sliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer)
	{
		if (!scene)
		{
			LOGM("CrSlice::sliceFromScene empty scene.");
			return;
		}

		cura52::Application app(tracer);
		app.tempDirectory = scene->m_tempDirectory;

		m_debugger.reset(new CacheDebugger(&app, scene));
		app.debugger = m_debugger.get();

		app.runCommulication(m_debugger.get());
	}

	trimesh::box3 CrSCacheSlice::groupBox()
	{
		trimesh::box3 b;
		b += convert(m_debugger->groupData._min);
		b += convert(m_debugger->groupData._max);
		return b;
	}

	int CrSCacheSlice::layers()
	{
		return (int)m_debugger->groupData.sliceLayers.size();
	}

	CacheLayer* CrSCacheSlice::createCacheLayer(int layer)
	{
		if (layer < 0 || layer >= layers())
			return nullptr;

		return new CacheLayer(layer, m_debugger.get());
	}
}