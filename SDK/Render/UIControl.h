#pragma once

#include <string>
#include <Dependencies/MemLib/Helpers.h>
#include <Dependencies/LibHat/libhat/Access.hpp>
#include <Util/Math.h>
#include <Util/Memory.h>

class UIControl {
public:
    BUILD_ACCESS(this, Vector2<float>, sizeConstrains, Offsets::UIControl_SizeConstrains);
    BUILD_ACCESS(this, Vector2<float>, parentRelativePosition, Offsets::UIControl_ParentRelativePosition);

    std::string& getLayerName() {
        return hat::member_at<std::string>(this, Offsets::UIControl_LayerName);
    }
};