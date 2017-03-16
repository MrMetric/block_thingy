#pragma once
#include "Base.hpp"

#include <map>

#include <msgpack.hpp>

namespace Block {

class Unknown : public Base
{
public:
	Unknown(BlockType);
	Unknown(BlockType, const std::string& strid);

	Unknown& operator=(const Base&) override;

	std::string name() const override;

	void save(Storage::OutputInterface&) const override;
	void load(Storage::InputInterface&) override;

private:
	const std::string strid;
	std::map<std::string, msgpack::object_handle> data;
};

}
