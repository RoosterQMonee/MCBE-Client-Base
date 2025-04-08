#include "SetupAndRender.h"

#include <Util/Memory.h>
#include <MCBase/MCBase.h>
#include <MemLib/Scanner.h>
#include <MemLib/Helpers.h>
#include <MemLib/MinHook/Minhook.h>
#include <MCBase/Events/Impl/GraphicalEvents.h>

void SetupAndRenderHook::Enable() {
	hat::scan_result result = hat::find_pattern(MemoryManager::GetSignature("SetupAndRender"), ".text");
	const std::byte* address = result.get();

	if (address == nullptr) {
		spdlog::warn("Failed to scan for hook: {}", m_name);
		return;
	}

	uintptr_t targetAddress = reinterpret_cast<uintptr_t>(address);
	try {
		MemoryManager::CreateHook<&SetupAndRenderHook::OnSetupAndRender>(targetAddress);
		spdlog::info("Hook installed successfully for {}", m_name);
	}
	catch (const std::exception& ex) {
		spdlog::error("Failed to hook {}: {}", m_name, ex.what());
	}
}

void SetupAndRenderHook::Disable() {
	MemoryManager::DisableHook<&SetupAndRenderHook::OnSetupAndRender>();
}

SetupAndRenderHook::SetupAndRenderHook() : Hook("SetupAndRender") {

}

SetupAndRenderHook::~SetupAndRenderHook() {
	Disable();
}

// --------------------------------------------------------------- //

void SetupAndRenderHook::OnSetupAndRender(ScreenView* pScreenView, MinecraftUIRenderContext* muirc) {
	Client.get()->m_clientInstance = muirc->GetClientInstance();
	Client.get()->m_dispatcher.trigger(SetupAndRenderEvent{
		pScreenView->VisualTree->root->getLayerName(),
		muirc->GetClientInstance(),
		pScreenView
	});
	MemLib::call_func<void, ScreenView*, MinecraftUIRenderContext*>(
		MemoryManager::GetOriginal<&SetupAndRenderHook::OnSetupAndRender>(),
		pScreenView, muirc
	);
}