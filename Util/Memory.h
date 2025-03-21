#pragma once

#include "Winrt.h"
#include <Dependencies/LibHat/libhat/Scanner.hpp>

#define DEFINE_SIGNATURE(pat, sig) constexpr hat::fixed_signature pat = hat::compile_signature<sig>();
#define DEFINE_OFFSET(off, val) constexpr uintptr_t off = val;


// TODO: Make multi-version support
namespace Signatures {
	DEFINE_SIGNATURE(FieldOfView, "f3 0f 10 48 ? 48 81 c1");
	DEFINE_SIGNATURE(DisplayClientMessage, "40 55 53 56 57 41 56 48 8D AC 24 A0 FE FF FF 48 81 EC 60 02 00 00 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 50 01 00 00 41");
	DEFINE_SIGNATURE(SetupAndRender, "48 8B C4 48 89 58 18 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 98 FD");
};

namespace Offsets {
	// VFuncs
	DEFINE_OFFSET(MinecraftUIRenderContext_ClientInstance, 200);
	DEFINE_OFFSET(ClientInstance_GetScreenName, 268);
	DEFINE_OFFSET(ClientInstance_GetLocalPlayer, 30);

	// Offsets
	DEFINE_OFFSET(ClientInstance_GuiData, 0x590);
}