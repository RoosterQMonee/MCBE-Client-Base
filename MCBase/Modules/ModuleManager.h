#pragma once

#include <Util/Manager.h>

#include "Module.h"

class ModuleManager : public Manager<Module> {
public:
	void Init() {
		this->AddMultiple<

		>();
	}
};