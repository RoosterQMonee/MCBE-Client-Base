#pragma once

#include <string>

class ClientInstance;
class ScreenView;

struct ImGuiRenderEvent {

};

struct SetupAndRenderEvent {
	std::string m_layerName;
	ClientInstance* m_clientInstance;
	ScreenView* m_screenView;
};