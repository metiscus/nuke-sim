#include "input.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include "logging.h"
#include "remotery.h"

namespace Input
{
	IOEventCallback on_mouse_motion;
	IOEventCallback on_mouse_button_down;
	IOEventCallback on_mouse_button_up;
	IOEventCallback on_keyboard_down;
	IOEventCallback on_keyboard_up;
	IOEventCallback on_keyboard_repeat;

	namespace 
	{
		void keyboard_callback(GLFWwindow* win, int key, int scancode, int action, int mods)
		{
			rmt_ScopedCPUSample(keyboard_callback, 0);
			IOEventCallback *callback = nullptr;

			InputEvent event;
			event.type = InputEvent::Keyboard;
			switch(action)
			{
				case GLFW_PRESS:   
					event.event = InputEvent::KeyDown;
					callback = &on_keyboard_down;
					break;

				case GLFW_RELEASE: 
					event.event = InputEvent::KeyUp; 
					callback = &on_keyboard_up;
					break;

				case GLFW_REPEAT:
					event.event = InputEvent::KeyRepeat;
					callback = &on_keyboard_repeat;
					break;
			}

			event.button = key;
			event.mods   = mods;

			if(callback && *callback)
			{
				(*callback)(event);
			}
		}

		void mouse_button_callback(GLFWwindow* win, int button, int action, int mods)
		{
			rmt_ScopedCPUSample(mouse_button_callback, 0);
			IOEventCallback *callback = nullptr;

			InputEvent event;
			event.type = InputEvent::MouseButton;
			switch(action)
			{
				case GLFW_PRESS:   
					event.event = InputEvent::MouseDown;
					callback = &on_mouse_button_down;
					break;

				case GLFW_RELEASE: 
					event.event = InputEvent::MouseUp; 
					callback = &on_mouse_button_up;
					break;
			}

			event.button = button;
			event.mods   = mods;

			if(callback && *callback)
			{
				(*callback)(event);
			}
		}

		void mouse_cursor_callback(GLFWwindow* win, double x, double y)
		{
			rmt_ScopedCPUSample(mouse_cursor_callback, 0);
			IOEventCallback *callback = nullptr;

			InputEvent event;
			event.type = InputEvent::MouseMotion;
			event.event = InputEvent::None;

			event.cursor_x = x;
			event.cursor_y = y;

			LOG_F(INFO, "input::mouse_cursor_callback");

			if(on_mouse_motion)
			{
				on_mouse_motion(event);
			}
		}
	}

	void initialize()
	{
		LOG_F(INFO, "Initializing Input subsystem");
		GLFWwindow* window = (GLFWwindow*)Window::get_native_handle();
		if(window)
		{
			glfwSetKeyCallback(window, keyboard_callback);
			glfwSetMouseButtonCallback(window, mouse_button_callback);
			glfwSetCursorPosCallback(window, mouse_cursor_callback);
		}
	}

	void update()
	{
		rmt_ScopedCPUSample(Input_update, 0);
		LOG_F(1, "Input::update()");
		glfwPollEvents();
	}
}