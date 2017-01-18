#pragma once

#include <cstdint>
#include "resource.h"
#include <memory>
#include <boost/iostreams/device/mapped_file.hpp>

class BinaryResource
	: public Resource
{
	uint64_t size_;

	std::string filename_;

	std::unique_ptr<boost::iostreams::mapped_file_source> data_;

public:
	BinaryResource()
	{
		set_type(BinaryResourceType);
	}

	~BinaryResource() = default;

	inline uint32_t get_size() const
	{
		return size_;
	}

	virtual uint64_t get_memory_usage() const
	{
		return size_;
	}

	inline const uint8_t* get_data() const
	{
		if(data_ && data_->is_open())
		{
			return (const uint8_t*)data_->data();
		}
	}

	virtual Guid load_file(const std::string& filename);
	virtual Guid load_file_as_guid(const std::string& filename, Guid guid);
};