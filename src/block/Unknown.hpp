#pragma once
#include "Base.hpp"

#include <map>

#include <msgpack.hpp>

namespace Block {

class Unknown : public Base
{
public:
	Unknown(BlockType);

	Unknown& operator=(const Base&) override;

	void save(Storage::OutputInterface&) const override;
	void load(Storage::InputInterface&) override;

private:
	std::map<std::string, msgpack::object_handle> data;
};

}
