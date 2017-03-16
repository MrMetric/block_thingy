#include "Unknown.hpp"

#include <easylogging++/easylogging++.hpp>

#include "storage/Interface.hpp"

namespace Block {

Unknown::Unknown(const BlockType t)
:
	Base(t)
{
}

Unknown& Unknown::operator=(const Base& block)
{
	Base::operator=(block);
	const Unknown* that = static_cast<const Unknown*>(&block);
	for(const auto& p : that->data)
	{
		data.emplace(p.first, Storage::copy_object(p.second.get()));
	}
	return *this;
}

void Unknown::save(Storage::OutputInterface& i) const
{
	// Base::save(i) not needed here, since the external ID is kept
	for(const auto& p : data)
	{
		i.set(p.first, p.second.get());
	}
}

void Unknown::load(Storage::InputInterface& i)
{
	Base::load(i); // does nothing yet; dunno if it will be needed later
	data = i.copy_all();
}

}
