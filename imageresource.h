#pragma once

#include <cstdint>
#include "resource.h"

class ImageResource
	: public Resource
{
	uint32_t width_;
	uint32_t height_;
	uint32_t channels_;

	std::vector<uint8_t> data_;
	std::string filename_;

public:
	ImageResource();
	~ImageResource();

	inline uint32_t get_width() const
	{
		return width_;
	}

	inline uint32_t get_height() const
	{
		return height_;
	}

	inline uint32_t get_channels() const
	{
		return channels_;
	}

	virtual uint32_t get_memory_usage() const
	{
		return width_ * height_ * channels_;
	}

	inline const uint8_t* get_data() const
	{
		return &data_[0];
	}

	virtual Guid load_file(const std::string& filename);
	virtual Guid load_file_as_guid(const std::string& filename, Guid guid);
};