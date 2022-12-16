#ifndef CXUTIL_CALLBACK_1607502965747_H
#define CXUTIL_CALLBACK_1607502965747_H
#include "cxutil/math/AABB3D.h"
#include "cxutil/settings/PrintFeature.h"
#include "slicer.h"

namespace cxutil
{
	class Mesh;
	class SliceCallback
	{
	public:
		virtual ~SliceCallback() {}

		virtual void onSceneBox(const AABB3D& box3) {};
		virtual void onLayerCount(int layer) {};
		virtual void onFilamentLen(double len) {};
		virtual void onPrintTime(int time) {};

		virtual void onLayerPart(int meshIdx, int layerIdx, int partIdx, float z, float thickness, ClipperLib::Path& path) {};
		virtual void onSupport(int layerIdx, int partIdx, float thickness, ClipperLib::Path& path) {};
        virtual void onSupports(int layerIdx, float thickness, ClipperLib::Paths& paths) {};
		virtual void onCxutilMesh(Mesh* mesh) {};

		//GCode
		virtual void onExtruderChanged(int index);
		virtual void onTotalLayers(int num);
		virtual void onWrite(float velocity, int x, int y, int z, PrintFeatureType type);
		virtual void onLayerStart(int layer, float thickness);
	};

	class DebugCallback
	{
	public:
		~DebugCallback() {}

		virtual void onZPaths(ClipperLib::Paths* apaths, float z, int type = 0)=0;
	};
}

#endif // CXUTIL_CALLBACK_1607502965747_H