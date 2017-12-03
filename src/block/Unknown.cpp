#include "Unknown.hpp"

#include "block/Enum/VisibilityType.hpp"
#include "storage/Interface.hpp"

using std::string;

namespace Block {

Unknown::Unknown(const Enum::Type t)
:
	Unknown(t, "")
{
}

Unknown::Unknown
(
	const Enum::Type t,
	const string& strid
)
:
	Base(t, Enum::VisibilityType::opaque),
	strid(strid)
{
}

Unknown& Unknown::operator=(const Base& block)
{
	Base::operator=(block);
	const Unknown& that = *static_cast<const Unknown*>(&block);
	strid = that.strid;
	for(const auto& p : that.data)
	{
		data.emplace(p.first, Storage::copy_object(p.second.get()));
	}
	return *this;
}

string Unknown::name() const
{
	if(strid.empty())
	{
		return "Unknown";
	}
	// TODO: return just strid in red
	const std::size_t entry_count = data.size() - 1;
	const string ending = (entry_count == 1) ? "y" : "ies";
	return "Unknown: " + strid + " (" + std::to_string(entry_count) + " entr" + ending + ")";
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
