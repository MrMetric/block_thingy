#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "fwd/block/base.hpp"
#include "fwd/block/enums/type.hpp"

namespace block_thingy::block {

class block_maker
{
public:
	block_maker() = default;
	block_maker(const block_maker&) = default;
	virtual ~block_maker() = default;
	virtual std::shared_ptr<base> make(enums::type) const;
};

template<typename T>
class block_maker_instance : public block_maker
{
public:
	std::shared_ptr<base> make(const enums::type t) const override
	{
		return std::make_shared<T>(t);
	}
};

class BlockRegistry
{
public:
	BlockRegistry();

	BlockRegistry(BlockRegistry&&) = delete;
	BlockRegistry(const BlockRegistry&) = delete;
	BlockRegistry& operator=(BlockRegistry&&) = delete;
	BlockRegistry& operator=(const BlockRegistry&) = delete;

	template<typename T, typename... Args>
	enums::type add(const std::string& strid, Args&&... args)
	{
		enums::type t = add_(strid, std::make_unique<block_maker_instance<T>>());
		default_blocks[t] = std::make_shared<T>(t, std::forward<Args>(args)...);
		return t;
	}

	std::shared_ptr<base> get_default(enums::type) const;
	std::shared_ptr<base> get_default(enums::type_external) const;
	std::shared_ptr<base> get_default(const std::string& strid) const;

	std::shared_ptr<base> make(enums::type) const;
	std::shared_ptr<base> make(enums::type_external) const;
	std::shared_ptr<base> make(const std::string& strid) const;
	std::shared_ptr<base> make(const std::shared_ptr<base>) const;

	enums::type get_id(const std::string& strid) const;
	//enums::type get_id(enums::type_external) const;

	std::string get_strid(enums::type) const;
	std::string get_strid(enums::type_external) const;

	std::string get_name(enums::type) const;
	std::string get_name(const std::string& strid) const;

	enums::type_external get_extid(enums::type) const;

	using extid_map_t = std::map<enums::type_external, std::string>;
	void reset_extid_map();
	void set_extid_map(extid_map_t);
	const extid_map_t& get_extid_map() const;

	enums::type_t get_max_id() const;

private:
	void make_strid_to_extid_map();
	enums::type add_(const std::string& strid, std::unique_ptr<block_maker>);

	mutable std::map<enums::type, std::shared_ptr<base>> default_blocks;
	mutable std::mutex default_blocks_mutex;
	std::map<enums::type, std::unique_ptr<block_maker>> block_makers;

	std::map<std::string, enums::type> strid_to_id;
	std::map<enums::type, std::string> id_to_strid;
	extid_map_t extid_to_strid;
	std::map<std::string, enums::type_external> strid_to_extid;
	enums::type_external max_extid;
};

}
