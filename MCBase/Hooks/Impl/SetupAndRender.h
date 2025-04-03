#pragma once

#include <MCBase/Hooks/Hook.h>
#include <SDK/Render/ScreenView.h>
#include <SDK/Render/MinecraftUIRenderContext.h>

class SetupAndRenderHook : public Hook {
public:
	SetupAndRenderHook();
	~SetupAndRenderHook();

	void Enable();
	void Disable();

private:
	static void OnSetupAndRender(ScreenView* pScreenView, MinecraftUIRenderContext* muirc);
};