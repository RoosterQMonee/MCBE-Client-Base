#pragma once

#include <Dependencies/MemLib/Helpers.h>
#include <Util/Memory.h>
#include "UIControl.h"

class VisualTree {
public:
    BUILD_ACCESS(this, class UIControl*, root, Offsets::VisualTree_Root);
};