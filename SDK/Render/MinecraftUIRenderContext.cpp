#include "MinecraftUIRenderContext.h"

#include <Util/Memory.h>
#include <Dependencies/LibHat/libhat/Access.hpp>

ClientInstance* MinecraftUIRenderContext::GetClientInstance() {
    return hat::member_at<ClientInstance*>(this, Offsets::MinecraftUIRenderContext_ClientInstance);
}