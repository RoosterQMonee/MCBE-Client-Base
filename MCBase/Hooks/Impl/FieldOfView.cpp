#include "FieldOfView.h"

#include <Util/Memory.h>
#include <MCBase/MCBase.h>
#include <MemLib/Scanner.h>
#include <MemLib/Helpers.h>
#include <MemLib/MinHook/Minhook.h>

void* target = nullptr;

void FieldOfViewHook::Enable() {
	hat::scan_result result = hat::find_pattern(Signatures::FieldOfView, ".text");
	const std::byte* address = result.get();

	if (address == nullptr) {
		spdlog::warn("Failed to scan for hook: {}", m_name);
		return;
	}

	// not a huge fan of this
	target = reinterpret_cast<void*>(const_cast<std::byte*>(address));

	MH_CreateHook(target, &FieldOfViewHook::OnFieldOfView, &m_original);
	MH_EnableHook(target);
}

void FieldOfViewHook::Disable() {
	if (target != nullptr)
		MH_DisableHook(target);
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
		m_original, self, fov, a
	);
}