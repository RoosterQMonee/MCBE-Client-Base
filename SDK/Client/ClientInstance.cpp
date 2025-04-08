#include "ClientInstance.h"

#include <Util/Memory.h>
#include <Dependencies/MemLib/Helpers.h>
#include <LibHat/libhat/Access.hpp>

GuiData* ClientInstance::GetGuiData() {
    return hat::member_at<GuiData*>(
        this, MemoryManager::Offsets<uintptr_t>::GetOffset("ClientInstance::GuiData"));
};

std::string ClientInstance::GetScreenName() {
    return MemLib::call_vfunc_index<std::string>(
        MemoryManager::Offsets<uintptr_t>::GetOffset("ClientInstance::GetScreenName"), this);
}

//LocalPlayer* ClientInstance::GetLocalPlayer() {
//    return MemLib::call_vfunc_index<LocalPlayer*>(Offsets::ClientInstance_GetLocalPlayer, this);
//}