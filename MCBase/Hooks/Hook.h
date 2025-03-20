#pragma once

#include <Dependencies/spdlog/spdlog.h>
#include <string>

class Hook {
public:
	Hook(std::string& name) : m_name{ name } {};
	Hook(const char* name) : m_name{ std::string(name) } {};

	virtual void Enable() { spdlog::debug("Enable() with no override ({})", m_name); };
	virtual void Disable() { spdlog::debug("Disable() with no override ({})", m_name); };

	std::string GetName() { return m_name; }

protected:
	std::string m_name;
	static inline void* m_original = nullptr;
};