#ifndef CRSLICE_CRGROUP_CREATE_1669515380929_H
#define CRSLICE_CRGROUP_CREATE_1669515380929_H
#include "Scene.h"
#include "crslice/crscene.h"

namespace cura52
{
	class Application;
}
namespace crslice
{
	cura52::Scene* createSliceFromCrScene(cura52::Application* application, CrScenePtr scene);
}

#endif // CRSLICE_CRGROUP_CREATE_1669515380929_H