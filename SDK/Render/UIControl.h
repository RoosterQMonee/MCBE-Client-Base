#pragma once

#include <string>
#include <Dependencies/MemLib/Helpers.h>
#include <Dependencies/LibHat/libhat/Access.hpp>
#include <Util/Math.h>
#include <Util/Memory.h>

class UIControl {
public:
    BUILD_ACCESS(this, Vector2<float>, sizeConstrains, MemoryManager::Offsets<uintptr_t>::GetOffset("UIControl::SizeConstrains"));
    BUILD_ACCESS(this, Vector2<float>, parentRelativePosition, MemoryManager::Offsets<uintptr_t>::GetOffset("UIControl::ParentRelativePosition"));

    std::string& getLayerName() {
        return hat::member_at<std::string>(this, MemoryManager::Offsets<uintptr_t>::GetOffset("UIControl::LayerName"));
    }
};