#pragma once
#include "Base.hpp"

#include <map>
#include <string>

#include <msgpack.hpp>

namespace block_thingy::block {

class Unknown : public Base
{
public:
	Unknown(enums::Type);
	Unknown(enums::Type, const std::string& strid);

	Unknown& operator=(const Base&) override;

	std::string name() const override;

	void save(storage::OutputInterface&) const override;
	void load(storage::InputInterface&) override;

private:
	std::string strid;
	std::map<std::string, msgpack::object_handle> data;
};

}
