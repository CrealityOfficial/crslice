#ifndef CX_SCENEINPUT_1600053607518_H
#define CX_SCENEINPUT_1600053607518_H
#include "cxutil/input/groupinput.h"
#include "cxutil/input/extruderinput.h"
#include "cxutil/settings/WipeScriptConfig.h"
#include "cxutil/settings/RetractionConfig.h"

namespace cxutil
{
	class SceneInput
	{
	public:
		SceneInput();
		~SceneInput();

		void addGroupInput(GroupInputPtr input);
		void removeGroupInput(GroupInputPtr input);
		void addExtruderInput(ExtruderInputPtr input);
		void setCurrentModelGroup(GroupInput* input);
		GroupInput* currentModelGroup();
		void loadGlobalSetting(const std::string& settingFile);
		void loadMesh(const std::string& meshFile);

		const std::vector<GroupInputPtr>& groups() const;
		const std::vector<ExtruderInputPtr>& extruders() const;
		std::vector<ExtruderInputPtr>& extruders();
		std::vector<WipeScriptConfig>& wipeScriptConfigs();
		std::vector<RetractionConfig>& retractionConfigs();
		std::vector<RetractionConfig>& switchRetractionConfigs();

		int extruderCount();

		void release();
		Settings* settings();
		SceneParam* param();

		void finalize();
		void fillParam();
		void releaseMeshData();
	protected:
		std::vector<GroupInputPtr> m_inputs;
		std::vector<ExtruderInputPtr> m_extruders;
		Settings* m_setting;
		SceneParam* m_param;

		GroupInput* m_currentGroupInput;

		std::vector<WipeScriptConfig> wipe_config_per_extruder;
		std::vector<RetractionConfig> retraction_config_per_extruder;
		std::vector<RetractionConfig> extruder_switch_retraction_config_per_extruder; 
	};

	typedef std::shared_ptr<SceneInput> SceneInputPtr;
}

#endif // CX_SCENEINPUT_1600053607518_H