#pragma once

#include <MCBase/Hooks/Hook.h>
#include <SDK/Render/MinecraftUIRenderContext.h>

class ScreenView;

class SetupAndRenderHook : public Hook {
public:
	SetupAndRenderHook();
	~SetupAndRenderHook();

	void Enable();
	void Disable();

private:
	static void OnSetupAndRender(ScreenView* pScreenView, MinecraftUIRenderContext* muirc);
};