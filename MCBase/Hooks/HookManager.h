#pragma once

#include <Util/Manager.h>

#include "Hook.h"
#include "Impl/DirectX.h"
#include "Impl/FieldOfView.h"

class HookManager : public Manager<Hook> {
public:
	void Init() {
		this->AddMultiple<
			DirectXHook,
			FieldOfViewHook
		>();
	}
};