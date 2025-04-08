#include "FieldOfView.h"

#include <Util/Memory.h>
#include <MCBase/MCBase.h>
#include <MemLib/Scanner.h>
#include <MemLib/Helpers.h>
#include <MemLib/MinHook/Minhook.h>

void FieldOfViewHook::Enable() {
	auto sig = MemoryManager::Signatures::GetSignature("FieldOfView");
	hat::scan_result result = hat::find_pattern(sig, ".text");
	const std::byte* address = result.get();

	if (address == nullptr) {
		spdlog::warn("Failed to scan for hook: {}", m_name);
		return;
	}

	uintptr_t targetAddress = reinterpret_cast<uintptr_t>(address);
	try {
		MemoryManager::Hooks<decltype(&FieldOfViewHook::OnFieldOfView)>::CreateHook<&FieldOfViewHook::OnFieldOfView>(targetAddress);
		spdlog::info("Hook installed successfully for {}", m_name);
	}
	catch (const std::exception& ex) {
		spdlog::error("Failed to hook {}: {}", m_name, ex.what());
	}
}

void FieldOfViewHook::Disable() {
	MemoryManager::Hooks<decltype(&FieldOfViewHook::OnFieldOfView)>::DisableHook<&FieldOfViewHook::OnFieldOfView>();
}

FieldOfViewHook::FieldOfViewHook() : Hook("FieldOfView") {

}

FieldOfViewHook::~FieldOfViewHook() {
	Disable();
}

// --------------------------------------------------------------- //

void* FieldOfViewHook::OnFieldOfView(LevelRendererPlayer* self, float fov, bool a) {
	if (m_fov != NULL)
		fov = m_fov;
	
	return MemLib::call_func<void*, LevelRendererPlayer*, float, bool>(
		MemoryManager::GetOriginal<&FieldOfViewHook::OnFieldOfView>(),
		self, fov, a
	);
}