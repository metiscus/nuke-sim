#include "window.h"

#include <GLFW/glfw3.h>

#include "assert.h"
#include "debugger.h"
#include "logging.h"
#include "input.h"

namespace Window
{
	namespace
	{
		bool initialize_ = false;
		GLFWwindow* window_ = nullptr;
	}

	WindowInfo info;

	void position_callback(GLFWwindow* window, int x, int y);
	void size_callback(GLFWwindow* window, int w, int h);
	assert_action assert_callback(const char* cond, const char* msg, const char* file, unsigned int line, void* user_data);


	void create_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const std::string& filename)
	{
		if(!initialize_)
		{
			LOG_F(INFO, "Initializing GLFW");
			if(!glfwInit())
			{
				ABORT_F("Failed to initialize GLFW");
			}
		}

		if(window_ == nullptr)
		{
			LOG_F(INFO, "Creating main window.");
			//TODO: When full screen is needed, add functionality here
			window_ = glfwCreateWindow(w, h, filename.c_str(), NULL, NULL);
			ASSERT(window_);

			if(window_)
			{
				glfwMakeContextCurrent(window_);
				glfwSetWindowPosCallback(window_, position_callback);
				glfwSetWindowSizeCallback(window_, size_callback);

				glfwSetWindowPos(window_, x, y);

#ifdef DEBUG
				assert_register_callback(assert_callback, nullptr);
#endif
			}
		}

		info.width = w;
		info.height = h;
		info.position_x = x;
		info.position_y = y;
	}

	void swap_buffer()
	{
		ASSERT(!!window_);
		glfwSwapBuffers(window_);
	}


	void position_callback(GLFWwindow* window, int x, int y)
	{
		ASSERT(!!window_);
		info.position_x = x;
		info.position_y = y;
	}

	void size_callback(GLFWwindow* window, int w, int h)
	{
		ASSERT(!!window_);
		info.width = w;
		info.height = h;
	}

	assert_action assert_callback(const char* cond, const char* msg, const char* file, unsigned int line, void* user_data)
	{
		if(debugger_present())
		{
			return ASSERT_ACTION_BREAK;
		}
		else {
			return ASSERT_ACTION_NONE;
		}
	}

	void* get_native_handle()
	{
		return window_;
	}
}