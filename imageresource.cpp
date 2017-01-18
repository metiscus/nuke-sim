#include "imageresource.h"
#include "private/stb_image.h"
#include "resourcemanager.h"

#include <mutex>
#include <memory>
#include <cstring>
#include "logging.h"

namespace 
{
	std::mutex image_loader_serializer;
}

Resource::Guid ImageResource::load_file(const std::string& filename)
{
	LOG_F(INFO, "Failed to load %s.", filename.c_str());
	return Resource::Guid();
}

Resource::Guid ImageResource::load_file_as_guid(const std::string& filename, Guid guid)
{
	int x, y, c;
	
	image_loader_serializer.lock();
	uint8_t *data = stbi_load(filename.c_str(), &x, &y, &c, 4);
	image_loader_serializer.unlock();
	if(data && (x > 0) && (y > 0))
	{
		auto image = std::make_shared<ImageResource>();
		image->data_.resize(x*y*4);

		memcpy(&image->data_[0], data, x*y*4);
		free(data);

		image->width_ = x;
		image->height_ = y;
		image->channels_ = c;

		LOG_F(INFO, "Loaded image %s [%d x %d].", filename.c_str(), x, y);

		set_is_loaded();

		return guid;
	}
	else
	{
		LOG_F(INFO, "Failed to load %s.", filename.c_str());
	}

	return Resource::Guid();
}