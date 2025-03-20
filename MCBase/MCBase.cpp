#include "MCBase.h"
#include <Util/Winrt.h>
#include <Dependencies/spdlog/spdlog.h>

// Key: 48 83 EC ? ? ? C1 4C 8D 05
// Mouse: 48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 44 0F B7 BC 24

void IClient::Init(HINSTANCE instance) {
	m_moduleInformation = MemLib::Module::get_module_information("Minecraft.Windows.exe");
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%^%H:%M:%S%$] %v");
    spdlog::info("Loading MCBase...");

    SetTitle("MCBase v{}", GetFormattedVersion());

    Client.get()->m_hookManager.Init();
    Client.get()->m_hookManager.ForEach([](Hook& hk) {
        hk.Enable();
        spdlog::info("Enabled Hook: {}", hk.GetName());
    });

    Client.get()->m_moduleManager.Init();
    Client.get()->m_moduleManager.ForEach([](Module& mod) {
        spdlog::info("Loaded Module: {}", mod.GetName());
    });

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}