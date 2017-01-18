#include "binaryresource.h"

#include <mutex>
#include <memory>
#include <cstring>
#include "logging.h"

namespace mfs = boost::iostreams;

Resource::Guid BinaryResource::load_file(const std::string& filename)
{
	return Resource::Guid();
}

Resource::Guid BinaryResource::load_file_as_guid(const std::string& filename, Guid guid)
{
	mfs::mapped_file_params params;
	params.path = filename;
	params.mode = std::ios_base::binary | std::ios_base::in;

	data_ = std::unique_ptr<mfs::mapped_file_source>( new mfs::mapped_file_source(params) );
	if(data_->is_open())
	{
		set_is_loaded();

		return guid;
	}
	else
	{
		LOG_F(INFO, "Failed to load %s.", filename.c_str());
	}

	return Resource::Guid();
}