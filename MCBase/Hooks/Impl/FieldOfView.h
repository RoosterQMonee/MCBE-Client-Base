#pragma once

#include <atomic>
#include <MCBase/Hooks/Hook.h>


class LevelRendererPlayer;

class FieldOfViewHook : public Hook {
public:
	static inline std::atomic<float> m_fov = 30;

	FieldOfViewHook();
	~FieldOfViewHook();

	void Enable();
	void Disable();

private:
	static void* OnFieldOfView(LevelRendererPlayer* self, float a, bool a2);
};