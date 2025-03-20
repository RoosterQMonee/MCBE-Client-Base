#pragma once

#include "Setting.h"
#include <memory>
#include <string>
#include <vector>

enum ModuleCategory : uint8_t {
	Client = 0,
	Visual,
	Player
};

class Module {
public:
	Module(std::string& name, ModuleCategory category) : m_name{ name }, m_category{ category } {};
	Module(const char* name, ModuleCategory category) : m_name{ std::string(name) }, m_category{ category } {};

	virtual void Enable() {};
	virtual void Disable() {};

	ModuleCategory GetCategory() { return m_category; }
	std::string GetName() { return m_name; }
	bool IsEnabled() { return m_enabled; }
	void SetEnabled(bool nstate) { m_enabled = nstate; }
	void Toggle() { m_enabled = !m_enabled; }

	template<typename T>
	void AddNewSetting(Setting<T>& setting) {
		m_settings.push_back(std::make_shared<FlexibleStorage<T>>(setting));
	}

	template<typename T>
	std::shared_ptr<Setting<T>> GetSetting(size_t index) {
		if (index >= m_settings.size()) {
			throw std::out_of_range("Setting index out of range");
		}
		return std::dynamic_pointer_cast<FlexibleStorage<T>>(m_settings[index]);
	}

private:
	std::vector<std::shared_ptr<Setting<void>>> m_settings;

protected:
	ModuleCategory m_category;
	std::string m_name;
	bool m_enabled;
};