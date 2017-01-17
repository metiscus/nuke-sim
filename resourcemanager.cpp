#include "resourcemanager.h"

#include "scheduler.h"
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include "logging.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


namespace ResourceManager
{
	namespace 
	{
    	// Data paths
    	std::vector<std::string> data_paths;

    	// Data
    	std::map<Resource::Guid, ResourcePtr> resources;
    	std::mutex resources_mutex;
	}

	void initialize()
	{
	}

	void shutdown()
	{
	}

	void add_data_path(const std::string& path)
	{
		data_paths.push_back(path);
	}

	Resource::Guid load_resource_file_impl(const std::string& file)
	{
		return Resource::Guid();
	}

	std::future<Resource::Guid> load_resource_file(const std::string& file)
	{
		std::promise<Resource::Guid> promise;
 
		fs::path target_path(file.c_str());
		if(!fs::exists(target_path))
		{
			LOG_F(INFO, "%s does not exist.", file.c_str());
			promise.set_value(Resource::Guid());
		}
		else
		{
			// Create a thread to process the load request
			std::thread( [&promise, file] 
			{ 
				promise.set_value_at_thread_exit(load_resource_file_impl(file));
			}).detach();
		}

		return promise.get_future();
	}

	ResourcePtr get_resource(Resource::Guid guid)
	{
		ResourcePtr ret;

		resources_mutex.lock();

		auto itr = resources.find(guid);
		if(itr != resources.end())
		{
			ret = itr->second;
		}

		resources_mutex.unlock();

		return ret;
	}

	std::future<ResourcePtr> get_resource_from_file(const std::string& file)
	{

	}
}