#pragma once

#include "Winrt.h"
#include <Dependencies/LibHat/libhat/Scanner.hpp>

#define DEFINE_SIGNATURE(pat, sig) constexpr hat::fixed_signature pat = hat::compile_signature<sig>();
#define DEFINE_OFFSET(off, val) constexpr uintptr_t off = val;


// TODO: Make multi-version support
namespace Signatures {
	DEFINE_SIGNATURE(FieldOfView, "f3 0f 10 48 ? 48 81 c1");
};