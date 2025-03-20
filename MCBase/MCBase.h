#pragma once

#include <Windows.h>
#include <memory>

#include <Dependencies/MemLib/Module.h>
#include <Dependencies/EnTT/entt.hpp>
#include <MCBase/Hooks/HookManager.h>
#include <MCBase/Modules/ModuleManager.h>
#include "Events/Impl/GraphicalEvents.h"
#include <ImGui/imgui.h>

class IClient {
public:
	MemLib::SectionHeader m_moduleInformation{};
	entt::dispatcher m_dispatcher;
	bool m_initialized;

	ModuleManager m_moduleManager;
	HookManager m_hookManager;

	void Init(HINSTANCE instance);
};

inline auto Client = std::make_shared<IClient>();