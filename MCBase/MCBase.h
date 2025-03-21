#pragma once

#include <Windows.h>
#include <memory>

#include <Dependencies/MemLib/Module.h>
#include <Dependencies/EnTT/entt.hpp>
#include <Dependencies/ImGui/imgui.h>
#include <MCBase/Hooks/HookManager.h>
#include <MCBase/Modules/ModuleManager.h>
#include <SDK/Client/ClientInstance.h>

class IClient {
public:
	MemLib::SectionHeader m_moduleInformation{};
	entt::dispatcher m_dispatcher;
	bool m_initialized;

	ModuleManager m_moduleManager;
	HookManager m_hookManager;

	ClientInstance* m_clientInstance;

	void Init(HINSTANCE instance);
};

inline auto Client = std::make_shared<IClient>();