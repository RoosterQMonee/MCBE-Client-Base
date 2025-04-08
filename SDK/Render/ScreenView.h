#pragma once

#include <Dependencies/MemLib/Helpers.h>
#include <Util/Memory.h>
#include "VisualTree.h"

class ScreenView {
public:
    BUILD_ACCESS(this, class VisualTree*, VisualTree, MemoryManager::Offsets<uintptr_t>::GetOffset("ScreenView::VisualTree"));
};