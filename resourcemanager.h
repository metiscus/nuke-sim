#pragma once

#include "resource.h"
#include <string>
#include <future>

namespace ResourceManager
{
	typedef std::shared_ptr<Resource> ResourcePtr;

	void initialize();
	void shutdown();

	void add_data_path(const std::string& path);

	std::future<Resource::Guid> load_resource_file(const std::string& file);

	ResourcePtr get_resource(Resource::Guid guid);
	std::future<ResourcePtr> get_resource_from_file(const std::string& file);
}