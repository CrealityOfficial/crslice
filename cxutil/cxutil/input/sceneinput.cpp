#include "cxutil/input/sceneinput.h"
#include "cxutil/util/jsonsettingsloader.h"

namespace cxutil
{
	SceneInput::SceneInput()
		:m_currentGroupInput(nullptr)
	{
		m_setting = new Settings();
		m_param = new SceneParam();
	}

	SceneInput::~SceneInput()
	{
		delete m_param;
		delete m_setting;
	}

	void SceneInput::addGroupInput(GroupInputPtr input)
	{
		m_inputs.push_back(input);
	}

	void SceneInput::removeGroupInput(GroupInputPtr input)
	{

	}

	void SceneInput::addExtruderInput(ExtruderInputPtr input)
	{
		m_extruders.push_back(input);
		input->settings()->setParent(m_setting);
	}

	void SceneInput::setCurrentModelGroup(GroupInput* input)
	{
		m_currentGroupInput = input;
	}

	GroupInput* SceneInput::currentModelGroup()
	{
		return m_currentGroupInput;
	}

	void SceneInput::loadGlobalSetting(const std::string& settingFile)
	{
		loadJsonSetting(settingFile.c_str(), m_setting);
	}

	void SceneInput::loadMesh(const std::string& meshFile)
	{
		if (!m_currentGroupInput)
		{
			addGroupInput(GroupInputPtr(new GroupInput()));
			m_currentGroupInput = &*m_inputs.back();
		}

		m_currentGroupInput->loadMesh(meshFile);
	}

	const std::vector<GroupInputPtr>& SceneInput::groups() const
	{
		return m_inputs;
	}

	const std::vector<ExtruderInputPtr>& SceneInput::extruders() const
	{
		return m_extruders;
	}

	std::vector< ExtruderInputPtr>& SceneInput::extruders()
	{
		return m_extruders;
	}

	std::vector<WipeScriptConfig>& SceneInput::wipeScriptConfigs()
	{
		return wipe_config_per_extruder;
	}

	std::vector<RetractionConfig>& SceneInput::retractionConfigs()
	{
		return retraction_config_per_extruder;
	}

	std::vector<RetractionConfig>& SceneInput::switchRetractionConfigs()
	{
		return extruder_switch_retraction_config_per_extruder;
	}

	int SceneInput::extruderCount()
	{
		return (int)m_extruders.size();
	}

	void SceneInput::release()
	{

	}

	Settings* SceneInput::settings()
	{
		return m_setting;
	}

	SceneParam* SceneInput::param()
	{
		return m_param;
	}

	void SceneInput::finalize()
	{
		m_param->initialize(this);
		for (GroupInputPtr inputPtr : m_inputs)
		{
			Settings* groupSettings = inputPtr->settings();
			groupSettings->setParent(m_setting);

			inputPtr->finalize();
		}
	}

	void SceneInput::fillParam()
	{
		wipe_config_per_extruder.resize(extruderCount());
		retraction_config_per_extruder.resize(extruderCount());
		extruder_switch_retraction_config_per_extruder.resize(extruderCount());

		for (size_t extruder_index = 0; extruder_index < extruderCount(); extruder_index++)
		{
			ExtruderParam* extruder = m_extruders.at(extruder_index)->param();
			WipeScriptConfig& wipe_config = wipe_config_per_extruder[extruder_index];

			wipe_config.retraction_enable = extruder->wipe_retraction_enable;
			wipe_config.retraction_config.distance = extruder->wipe_retraction_amount;
			wipe_config.retraction_config.speed = extruder->wipe_retraction_retract_speed;
			wipe_config.retraction_config.primeSpeed = extruder->wipe_retraction_prime_speed;
			wipe_config.retraction_config.prime_volume = extruder->wipe_retraction_extra_prime_amount;
			wipe_config.retraction_config.retraction_min_travel_distance = 0;
			wipe_config.retraction_config.retraction_extrusion_window = std::numeric_limits<double>::max();
			wipe_config.retraction_config.retraction_count_max = std::numeric_limits<size_t>::max();

			wipe_config.pause = extruder->wipe_pause;

			wipe_config.hop_enable = extruder->wipe_hop_enable;
			wipe_config.hop_amount = extruder->wipe_hop_amount;
			wipe_config.hop_speed = extruder->wipe_hop_speed;

			wipe_config.brush_pos_x = extruder->wipe_brush_pos_x;
			wipe_config.repeat_count = extruder->wipe_repeat_count;
			wipe_config.move_distance = extruder->wipe_move_distance;
			wipe_config.move_speed = extruder->speed_travel;
			wipe_config.max_extrusion_mm3 = extruder->max_extrusion_before_wipe;
			wipe_config.clean_between_layers = extruder->clean_between_layers;

			RetractionConfig& retraction_config = retraction_config_per_extruder[extruder_index];
			retraction_config.distance = (extruder->retraction_enable) ? extruder->retraction_amount : 0;

			retraction_config.prime_volume = extruder->retraction_extra_prime_amount;
			retraction_config.speed = extruder->retraction_retract_speed;
			retraction_config.primeSpeed = extruder->retraction_prime_speed;
			retraction_config.zHop = extruder->retraction_hop;
			retraction_config.retraction_min_travel_distance = extruder->retraction_min_travel;
			retraction_config.retraction_extrusion_window = extruder->retraction_extrusion_window;
			retraction_config.retraction_count_max = extruder->retraction_count_max;

			RetractionConfig& switch_retraction_config = extruder_switch_retraction_config_per_extruder[extruder_index];
			switch_retraction_config.distance = extruder->switch_extruder_retraction_amount;
			switch_retraction_config.prime_volume = 0.0;
			switch_retraction_config.speed = extruder->switch_extruder_retraction_speed;
			switch_retraction_config.primeSpeed = extruder->switch_extruder_prime_speed;
			switch_retraction_config.zHop = extruder->retraction_hop_after_extruder_switch_height;
			switch_retraction_config.retraction_min_travel_distance = 0;
			switch_retraction_config.retraction_extrusion_window = 99999.9;
			switch_retraction_config.retraction_count_max = 9999999;
		}
	}

	void SceneInput::releaseMeshData()
	{
		for (GroupInputPtr inputPtr : m_inputs)
			inputPtr->releaseMeshData();
	}
}