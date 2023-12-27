#ifndef GCODEPROCESSHEADER_1632383314974_H
#define GCODEPROCESSHEADER_1632383314974_H

#include "ccglobal/tracer.h"
#include "trimesh2/TriMesh.h"
#include "trimesh2/XForm.h"

namespace gcode
{
    	enum class GCodeVisualType
	{
		gvt_speed,
		gvt_structure,
		gvt_extruder,
		gvt_layerHight,  //���
		gvt_lineWidth,   //�߿�
		gvt_flow,        //����
		gvt_layerTime,   //��ʱ��
		gvt_fanSpeed,    //�����ٶ�
		gvt_temperature, //�¶�
		gvt_num,
	};

	struct TimeParts {
		float OuterWall{0.0f};
		float InnerWall{ 0.0f };
		float Skin{ 0.0f };
		float Support{ 0.0f };
		float SkirtBrim{ 0.0f };
		float Infill{ 0.0f };
		float SupportInfill{ 0.0f };
		float MoveCombing{ 0.0f };
		float MoveRetraction{ 0.0f };
		float PrimeTower{ 0.0f };
	};

	struct GCodeParseInfo {
		float machine_height;
		float machine_width;
		float machine_depth;
		int printTime;
		float materialLenth;
		float materialDensity;//��λ����ܶ�
		float material_diameter = {1.75f}; //����ֱ��
		float material_density = { 1.24f };  //�����ܶ�
		float lineWidth;
		float layerHeight = {0.0f};
		float unitPrice;
		bool spiralMode;
		bool adaptiveLayers;
		std::string exportFormat;//QString exportFormat;
		std::string	screenSize;//QString screenSize;

		TimeParts timeParts;
	
		int beltType;  // 1 creality print belt  2 creality slicer belt
		float beltOffset;
		float beltOffsetY;
		trimesh::fxform xf4;//cr30 fxform
	
		bool relativeExtrude;
	
		GCodeParseInfo()
		{
			machine_height = 250.0f;
			machine_width = 220.0f;
			machine_depth = 220.0f;
			printTime = 0;
			materialLenth = 0.0f;
			materialDensity = 1.0f;
			lineWidth = 0.1f;
			layerHeight = 0.1f;
			unitPrice = 0.3f;
			exportFormat = "png";
			screenSize = "Sermoon D3";
			spiralMode = false;

			timeParts = TimeParts();
	
			beltType = 0;
			beltOffset = 0.0f;
			beltOffsetY = 0.0f;
			xf4 = trimesh::fxform();
			relativeExtrude = false;
			adaptiveLayers = false;
		}
	};

	class GcodeTracer
	{
	public:
		virtual ~GcodeTracer() {}

		virtual void tick(const std::string& tag) = 0;
		virtual void getPathData(const trimesh::vec3 point, float e, int type) = 0;
		virtual void getPathDataG2G3(const trimesh::vec3 point, float i, float j, float e, int type, bool isG2 = true) = 0;
		virtual void setParam(GCodeParseInfo& pathParam) = 0;
		virtual void setLayer(int layer) = 0;
		virtual void setLayers(int layer) = 0;
		virtual void setSpeed(float s) = 0;
		virtual void setTEMP(float temp) = 0;
		virtual void setExtruder(int nr) = 0;
		virtual void setTime(float time) = 0;
		virtual void setFan(float fan) = 0;
		virtual void setZ(float z, float h = -1) = 0;
		virtual void setE(float e) = 0;
		virtual void getNotPath() = 0;
		virtual void set_data_gcodelayer(int layer, const std::string& gcodelayer) = 0;
		virtual void setNozzleColorList(std::string& colorList) = 0;
	};


    class  SliceLine3D
    {
    public:
        SliceLine3D() {};
        ~SliceLine3D() {};

        trimesh::vec3 start;
        trimesh::vec3 end;
    };

    enum class SliceCompany
    {
        none,creality, cura, prusa, bambu, ideamaker, superslicer, ffslicer, simplify
    };

}


#endif // GCODEPROCESSHEADER_1632383314974_H