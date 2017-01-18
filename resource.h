#pragma once

#include <atomic>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <cstdint>
#include <string>
#include <vector>

enum ResourceType
{
	InvalidResourceType = 0,
	ImageResourceType   = 1,
	BinaryResourceType  = 2,
};

class Resource
{
public:
	typedef boost::uuids::uuid Guid;
	static inline Guid random_guid()
	{
		return boost::uuids::random_generator()();
	}

private:
	ResourceType type_;
	std::string name_;
	Guid guid_;
	std::atomic_bool is_loaded_;

	std::vector<Guid> dependencies_;

public:
	Resource()
		: type_ (InvalidResourceType)
		, name_ ("")
		, is_loaded_(false)
	{
		;
	}

	virtual ~Resource() = default;

	inline void set_name(const std::string& name) 
	{ 
		name_ = name;
	}

	inline void set_type(const ResourceType type)
	{
		type_ = type;
	}

	inline void set_guid(const Guid& guid)
	{
		guid_ = guid;
	}

	inline void add_dependency(const Guid& guid)
	{
		dependencies_.push_back(guid);
	}

	inline std::string get_name() const
	{
		return name_;
	}

	inline ResourceType get_type() const
	{
		return type_;
	}

	inline const Guid& get_guid() const
	{
		return guid_;
	}

	inline bool operator=(const Resource& rhs) const
	{
		return guid_ == rhs.guid_;
	}

	inline std::vector<Guid> get_dependencies() const
	{
		return dependencies_;
	}

	inline bool is_valid() const
	{
		return !guid_.is_nil();
	}

	inline bool is_loaded() const
	{
		return is_loaded_;
	}

	virtual uint64_t get_memory_usage() const = 0;
	virtual Guid load_file(const std::string& filename) = 0;
	virtual Guid load_file_as_guid(const std::string& filename, Guid guid) = 0;

protected:
	inline void set_is_loaded()
	{
		is_loaded_ = true;
	}
};