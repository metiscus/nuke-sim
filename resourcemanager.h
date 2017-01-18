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
	std::future<Resource::Guid> load_file_as_resource(const std::string& file, Resource::Guid guid, ResourceType type);

	void unload_resource(Resource::Guid guid);

	ResourcePtr get_resource(Resource::Guid guid);
	std::future<ResourcePtr> get_resource_from_file(const std::string& file);
}