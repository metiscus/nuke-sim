#include "resourcemanager.h"

#include <atomic>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <cstdlib>
#include <cstring>
#include "logging.h"
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "resource.h"
#include "imageresource.h"
#include "binaryresource.h"

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

    	std::atomic_bool initialized;
	}

	void initialize()
	{
		initialized = true;
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
		initialized = false;
	}

	void add_data_path(const std::string& path)
	{
		if(!initialized)
		{
			data_paths.push_back(path);
		}
		else
		{
			LOG_F(WARNING, "Ignoring data path add after initialization.");
		}
	}

	//Note: Anonymous namespaces hide symbols from being exported and are slightly
	// cleaner than just declaring the function static
	namespace {
		//Note: This was blatantly copied from the library documentation
		bool find_file(const fs::path & dir_path, const std::string & file_name, fs::path & path_found)
		{
		 	if ( !fs::exists( dir_path ) ) 
		 	{
		 		return false;
		 	}

		  	fs::recursive_directory_iterator end_itr;
		  	for ( fs::recursive_directory_iterator itr( dir_path ); itr != end_itr; ++itr )
		  	{
				if ( itr->path().filename() == file_name )
		    	{
		      		path_found = itr->path();
		      		return true;
		    	}
		  	}
		  	return false;
		}
	}

	std::string find_resource_file(const std::string& file)
	{
		try {
			// use boost::filesystem to search all of the paths and return the first match
			bool found_path = false;
			fs::path path;
			
			LOG_F(INFO, "Finding resource file '%s'.", file.c_str());
			for(auto itr = data_paths.begin(); !found_path && itr != data_paths.end(); ++itr)
			{
				LOG_F(INFO + 1, "Searching path '%s'.", itr->c_str());
				found_path = find_file(*itr, file, path);
			}

			if(found_path)
			{
				LOG_F(INFO, "Found path to resource file '%s'.", path.string().c_str());
				return path.string();
			}
			else
			{
				LOG_F(WARNING, "Unable to find a path to file '%s'. Assuming CWD.", file.c_str());
				return file;
			}
		}
		catch(fs::filesystem_error e)
		{
			LOG_F(ERROR, e.what());
			return file;
		}
	}

	Resource::Guid load_file_as_resource_impl(const std::string& file, Resource::Guid guid, ResourceType type = InvalidResourceType)
	{
		switch(type)
		{
			case InvalidResourceType:
			{
				LOG_F(WARNING, "Invalid resource type indicated for file '%s'.", file.c_str());
			}
			break;

			case ImageResourceType:
			{
				auto resource = std::make_shared<ImageResource>();
				if(!resource->load_file_as_guid(file, guid).is_nil())
				{
					resources_mutex.lock();
					resources.insert(std::make_pair(guid, resource));
					resources_mutex.unlock();
					return guid;
				}
			}
			break;

			case BinaryResourceType:
			{
				auto resource = std::make_shared<BinaryResource>();
				if(!resource->load_file_as_guid(file, guid).is_nil())
				{
					resources_mutex.lock();
					resources.insert(std::make_pair(guid, resource));
					resources_mutex.unlock();
					return guid;
				}
			}
			break;
		}
		return Resource::Guid();
	}

	Resource::Guid load_resource_file_impl(const std::string& file, ResourceType type = InvalidResourceType)
	{
		switch(type)
		{
			case InvalidResourceType:
			{
				LOG_F(WARNING, "Invalid resource type indicated for file '%s'.", file.c_str());
			}
			break;

			case ImageResourceType:
			{
				LOG_F(WARNING, "Image files do not contain GUIDs, so you must call load_resource_file_as_guid_impl for file '%s'.", file.c_str());
			}
			break;
		}
		return Resource::Guid();
	}

	std::future<Resource::Guid> load_resource_file(const std::string& file)
	{
		std::promise<Resource::Guid> promise;
 
		std::string file_path = find_resource_file(file);

		fs::path target_path(file_path.c_str());
		if(!fs::exists(target_path))
		{
			LOG_F(WARNING, "%s does not exist.", file.c_str());
			promise.set_value(Resource::Guid());
		}
		else
		{
			// Create a thread to process the load request
			std::thread( [&promise, file_path] 
			{ 
				promise.set_value_at_thread_exit(load_resource_file_impl(file_path));
			}).detach();
		}

		return promise.get_future();
	}

	std::future<Resource::Guid> load_file_as_resource(const std::string& file, Resource::Guid guid, ResourceType type)
	{
		std::promise<Resource::Guid> promise;
 
		std::string file_path = find_resource_file(file);

		fs::path target_path(file_path.c_str());
		if(!fs::exists(target_path))
		{
			LOG_F(WARNING, "%s does not exist.", file.c_str());
			promise.set_value(Resource::Guid());
		}
		else
		{
			// Create a thread to process the load request
			return std::async(std::launch::async, [file_path, guid, type] 
			{ 
				return (load_file_as_resource_impl(file_path, guid, type));
			});
		}

		return promise.get_future();
	}

	void unload_resource(Resource::Guid guid)
	{
		resources_mutex.lock();
		
		auto itr = resources.find(guid);
		if(itr != resources.end())
		{
			resources.erase(itr);
		}

		resources_mutex.unlock();
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