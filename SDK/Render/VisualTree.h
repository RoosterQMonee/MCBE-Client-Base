#pragma once

#include <Dependencies/MemLib/Helpers.h>
#include <Util/Memory.h>
#include "UIControl.h"

class VisualTree {
public:
    BUILD_ACCESS(this, class UIControl*, root, MemoryManager::Offsets<uintptr_t>::GetOffset("VisualTree::Root"));
};