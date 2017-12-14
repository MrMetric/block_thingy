#include "Unknown.hpp"

#include "block/Enum/VisibilityType.hpp"
#include "storage/Interface.hpp"

using std::string;

namespace block_thingy::block {

Unknown::Unknown(const enums::Type t)
:
	Unknown(t, "")
{
}

Unknown::Unknown
(
	const enums::Type t,
	const string& strid
)
:
	Base(t, enums::VisibilityType::opaque),
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
		data.emplace(p.first, storage::copy_object(p.second.get()));
	}
	return *this;
}

string Unknown::name() const
{
	if(strid.empty())
	{
		return Base::name();
	}
	// TODO: return just strid in red
	const std::size_t entry_count = data.size() - 1;
	const string ending = (entry_count == 1) ? "y" : "ies";
	return Base::name() + " (" + strid + ", " + std::to_string(entry_count) + " entr" + ending + ')';
}

void Unknown::save(storage::OutputInterface& i) const
{
	// Base::save(i) not needed here, since the external ID is kept
	for(const auto& p : data)
	{
		i.set(p.first, p.second.get());
	}
}

void Unknown::load(storage::InputInterface& i)
{
	Base::load(i); // does nothing yet; dunno if it will be needed later
	data = i.copy_all();
}

}
