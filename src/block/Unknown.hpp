#pragma once
#include "Base.hpp"

#include <map>
#include <string>

#include <msgpack.hpp>

namespace Block {

class Unknown : public Base
{
public:
	Unknown(Enum::Type);
	Unknown(Enum::Type, const std::string& strid);

	Unknown& operator=(const Base&) override;

	std::string name() const override;

	void save(Storage::OutputInterface&) const override;
	void load(Storage::InputInterface&) override;

private:
	std::string strid;
	std::map<std::string, msgpack::object_handle> data;
};

}
