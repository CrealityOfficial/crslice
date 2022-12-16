#include "cxutil/input/dlpdata.h"

namespace cxutil
{
	DLPData::DLPData()
	{
	}
	DLPData::~DLPData()
	{
		for (std::vector<DLPmeshs>::iterator it= m_dlpmeshsgroup.dlpmeshsgroup.begin(); it!= m_dlpmeshsgroup.dlpmeshsgroup.end();it++)
		{
			for (std::vector<DLPmesh>::iterator it2 = it->dlpmeshs.begin();it2!= it->dlpmeshs.end();it2++)
			{
				for (std::vector<DLPLayer>::iterator it3= it2->layers.begin(); it3!=it2->layers.end();it3++)
				{
					for (std::vector< ClipperLib::PolyTree*>::iterator it4=it3->parts.begin();it4!=it3->parts.end();it4++)
					{
						if (*it4 )
						{
							delete* it4;
							*it4 = nullptr;
						}
					}
					std::vector< ClipperLib::PolyTree*> tmp;
					it3->parts.swap(tmp);
				}
				std::vector<DLPLayer> dlplayer;
				it2->layers.swap(dlplayer);
			}
		}	
	}

	void DLPData::clearLayer(int layer)
	{
		for (std::vector<DLPmeshs>::iterator it = m_dlpmeshsgroup.dlpmeshsgroup.begin(); it != m_dlpmeshsgroup.dlpmeshsgroup.end(); it++)
		{
			for (std::vector<DLPmesh>::iterator it2 = it->dlpmeshs.begin(); it2 != it->dlpmeshs.end(); it2++)
			{
				auto& it3 = it2->layers.at(layer);
				{
					for (std::vector< ClipperLib::PolyTree*>::iterator it4 = it3.parts.begin(); it4 != it3.parts.end(); it4++)
					{
						if (*it4)
						{
							delete* it4;
							*it4 = nullptr;
						}
					}
					std::vector< ClipperLib::PolyTree*> ptree;		
					it3.parts=ptree;
				}
			}
		}
	}
	std::vector<ClipperLib::PolyTree*> DLPData::trait(int layer)
	{
		std::vector<cxutil::DLPmeshs>& meshGroup = m_dlpmeshsgroup.dlpmeshsgroup;
		std::vector<ClipperLib::PolyTree*> trees;
		for (cxutil::DLPmeshs& group : meshGroup)
		{
			std::vector<cxutil::DLPmesh>& meshes = group.dlpmeshs;

			for (cxutil::DLPmesh& mesh : meshes)
			{
				if (layer < (int)mesh.layers.size())
				{
					for (ClipperLib::PolyTree* tree : mesh.layers.at(layer).parts)
						trees.push_back(tree);
				}
			}
		}
		return trees;
	}

	int DLPData::layers()
	{
		int layer = 0;
		std::vector<cxutil::DLPmeshs>& meshGroup = m_dlpmeshsgroup.dlpmeshsgroup;
		for (cxutil::DLPmeshs& group : meshGroup)
		{
			std::vector<cxutil::DLPmesh>& meshes = group.dlpmeshs;

			for (cxutil::DLPmesh& mesh : meshes)
			{
				std::vector<cxutil::DLPLayer>& layers = mesh.layers;
				if (layers.size() > 0 && layer < layers.size())
					layer = (int)layers.size();
			}
		}
		return layer;
	}
}
