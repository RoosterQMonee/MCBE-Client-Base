#include "SetupAndRender.h"

#include <Util/Memory.h>
#include <MCBase/MCBase.h>
#include <MemLib/Scanner.h>
#include <MemLib/Helpers.h>
#include <MemLib/MinHook/Minhook.h>
#include <MCBase/Events/Impl/GraphicalEvents.h>

void SetupAndRenderHook::Enable() {
	hat::scan_result result = hat::find_pattern(Signatures::SetupAndRender, ".text");
	const std::byte* address = result.get();

	if (address == nullptr) {
		spdlog::warn("Failed to scan for hook: {}", m_name);
		return;
	}

	// not a huge fan of this
	target = reinterpret_cast<void*>(const_cast<std::byte*>(address));

	MH_CreateHook(target, &SetupAndRenderHook::OnSetupAndRender, &m_original);
	MH_EnableHook(target);
}

void SetupAndRenderHook::Disable() {
	if (target != nullptr)
		MH_DisableHook(target);
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
		m_original, pScreenView, muirc
	);
}