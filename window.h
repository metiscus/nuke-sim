#pragma once

#include <cstdint>
#include <string>

namespace Window
{

	struct WindowInfo
	{
		uint32_t width;
		uint32_t height;

		uint32_t position_x;
		uint32_t position_y;
	};


	extern WindowInfo info;

	void create_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const std::string& filename);

	void swap_buffer();

	void* get_native_handle();
}