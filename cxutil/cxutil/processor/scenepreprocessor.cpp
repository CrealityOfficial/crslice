#include "scenepreprocessor.h"

#include "cxutil/input/groupinput.h"
namespace cxutil
{
	bool checkMeshGroupValid(GroupInput* meshGroup)
	{
        MeshGroupParam* param = meshGroup->param();

        bool empty = true;
        for (const MeshInputPtr& mesh : meshGroup->meshes())
        {
            MeshParam* mParam = mesh->param();
            if (!mParam->isInfill() && !mParam->isAntiOverhang())
            {
                empty = false;
                break;
            }
        }

        if (empty)
            return false;

        //check parameter
        if (param->layer_height_0 <= 0)
        {
            return false;
        }

        if (param->layer_height <= 0)
        {
            return false;
        }

        return true;
	}
}