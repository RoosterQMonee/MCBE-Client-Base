#pragma once

#include <string>
#include <SDK/Render/GuiData.h>

class ClientInstance {
public:
    GuiData* GetGuiData();
    std::string GetScreenName();
    //LocalPlayer* GetLocalPlayer();
};