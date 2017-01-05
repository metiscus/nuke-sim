#include <functional>

#pragma once

namespace Input
{
	struct InputEvent
	{
		enum Type
		{
			Keyboard,
			MouseMotion,
			MouseButton
		};

		enum Event
		{
			KeyDown,
			KeyUp,
			KeyRepeat,
			MouseDown,
			MouseUp,
			None,
		};

		Type type;
		Event event;
		int32_t button;
		int32_t mods;
		double cursor_x;
		double cursor_y;
	};

	typedef std::function<void(InputEvent)> IOEventCallback;

	extern IOEventCallback on_mouse_motion;
	extern IOEventCallback on_mouse_button_down;
	extern IOEventCallback on_mouse_button_up;
	extern IOEventCallback on_keyboard_down;
	extern IOEventCallback on_keyboard_up;
	extern IOEventCallback on_keyboard_repeat;

	void initialize();

	void update();
}