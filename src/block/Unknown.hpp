#pragma once
#include "Base.hpp"

#include <map>
#include <string>

#include <msgpack.hpp>

namespace block_thingy::block {

class unknown : public base
{
public:
	unknown(enums::type);
	unknown(enums::type, const std::string& strid);

	unknown& operator=(const base&) override;

	std::string name() const override;

	void save(storage::OutputInterface&) const override;
	void load(storage::InputInterface&) override;

private:
	std::string strid;
	std::map<std::string, msgpack::object_handle> data;
};

}
