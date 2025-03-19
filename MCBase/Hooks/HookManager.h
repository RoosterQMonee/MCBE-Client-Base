#pragma once

#include <concepts>
#include <memory>
#include <Util/Manager.h>

#include "Hook.h"
#include "Impl/DirectX.h"

class HookManager : public Manager<Hook> {
public:
	void Init() {
		this->AddMultiple<
			DirectXHook
		>();
	}
};