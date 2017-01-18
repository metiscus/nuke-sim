#include "resourcemanager.h"

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <cstdlib>
#include <cstring>
#include "logging.h"
#include <map>
#include <memory>
#include <mutex>
#include <string>

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
		LOG_F(INFO, "Initializing ResourceManager");
		const boost::char_separator<char> delim("; ");
		const char* data_path_var = getenv("RESOURCE_MANAGER_DATA_PATH");
		if(data_path_var != nullptr)
		{
			std::string path = data_path_var;
			boost::tokenizer< boost::char_separator<char> > tokens(path, delim);
			for(const auto& token : tokens)
			{
				LOG_F(INFO, "Adding data path '%s'.", token.c_str());
				data_paths.push_back(token);
			}
		}
	}

	void shutdown()
	{
		//TODO: clean up allocated resources
		LOG_F(ERROR, "Shutdown currently unimplemented.");
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
		// Spawn a thread to load the resource file, return the future created 
		return std::async(std::launch::async, [file] () -> ResourcePtr
		{
			Resource::Guid guid = load_resource_file_impl(file);
			if(guid.is_nil())
			{
				LOG_F(INFO, "Loading %s returned a nil GUID.", file.c_str());
				return ResourcePtr();
			}
			else
			{
				return get_resource(guid);
			}
		});
	}
}