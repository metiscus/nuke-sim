#include <functional>

#pragma once

namespace Input
{
	enum KeyboardKey
	{
		Key_Space = 32,
		Key_0 = '0',
		Key_1 = '1',
		Key_2 = '2',
		Key_3 = '3',
		Key_4 = '4',
		Key_5 = '5',
		Key_6 = '6',
		Key_7 = '7',
		Key_8 = '8',
		Key_9 = '9',
		Key_A = 'A',
		Key_B = 'B',
		Key_C = 'C',
		Key_D = 'D',
		Key_E = 'E',
		Key_F = 'F',
		Key_G = 'G',
		Key_H = 'H',
		Key_I = 'I',
		Key_J = 'J',
		Key_K = 'K',
		Key_L = 'L',
		Key_M = 'M',
		Key_N = 'N',
		Key_O = 'O',
		Key_P = 'P',
		Key_Q = 'Q',
		Key_R = 'R',
		Key_S = 'S',
		Key_T = 'T',
		Key_U = 'U',
		Key_V = 'V',
		Key_W = 'W',
		Key_X = 'X',
		Key_Y = 'Y',
		Key_Z = 'Z',
	};

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