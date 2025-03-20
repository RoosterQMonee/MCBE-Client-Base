#include "MCBase.h"
#include <Dependencies/spdlog/spdlog.h>

void IClient::Init(HINSTANCE instance) {
	m_moduleInformation = MemLib::Module::get_module_information("Minecraft.Windows.exe");
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%^%H:%M:%S%$] %v");

    spdlog::info("Loading MCBase...");

    Client.get()->m_hookManager.Init();
    Client.get()->m_hookManager.ForEach([](Hook& hk) {
        hk.Enable();
        spdlog::info("Enabled {}", hk.m_name);
    });

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}