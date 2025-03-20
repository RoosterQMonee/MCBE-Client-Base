#pragma once

#include "Setting.h"
#include <memory>
#include <string>
#include <vector>

enum ModuleCategory : uint8_t {
	Visual = 0,
	Player
};

class Module {
public:
	Module(std::string& name, ModuleCategory category, bool enabled = false) : m_name{ name }, m_category{ category }, m_enabled{ enabled } {};
	Module(const char* name, ModuleCategory category, bool enabled = false) : m_name{ std::string(name) }, m_category{ category }, m_enabled{ enabled } {};

	virtual void Enable() {};
	virtual void Disable() {};

	ModuleCategory GetCategory() { return m_category; }
	std::string GetName() { return m_name; }
	bool IsEnabled() { return m_enabled; }
	void SetEnabled(bool nstate) { m_enabled = nstate; }
	void Toggle() { m_enabled = !m_enabled; }

	template<typename T>
	std::shared_ptr<Setting<T>> AddSetting(Setting<T>& setting) {
		auto setting = std::make_shared<Setting<T>>(setting);
		m_settings.push_back(setting);
		return setting;
	}

private:
	std::vector<std::shared_ptr<Setting<void>>> m_settings;

protected:
	ModuleCategory m_category;
	std::string m_name;
	bool m_enabled;
};