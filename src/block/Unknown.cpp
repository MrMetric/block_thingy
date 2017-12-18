#include "Unknown.hpp"

#include "block/Enum/VisibilityType.hpp"
#include "storage/Interface.hpp"

using std::string;

namespace block_thingy::block {

unknown::unknown(const enums::type t)
:
	unknown(t, "")
{
}

unknown::unknown
(
	const enums::type t,
	const string& strid
)
:
	base(t, enums::visibility_type::opaque),
	strid(strid)
{
}

unknown& unknown::operator=(const base& block)
{
	base::operator=(block);
	const unknown& that = *static_cast<const unknown*>(&block);
	strid = that.strid;
	for(const auto& p : that.data)
	{
		data.emplace(p.first, storage::copy_object(p.second.get()));
	}
	return *this;
}

string unknown::name() const
{
	if(strid.empty())
	{
		return base::name();
	}
	// TODO: return just strid in red
	const std::size_t entry_count = data.size() - 1;
	const string ending = (entry_count == 1) ? "y" : "ies";
	return base::name() + " (" + strid + ", " + std::to_string(entry_count) + " entr" + ending + ')';
}

void unknown::save(storage::OutputInterface& i) const
{
	// base::save(i) not needed here, since the external ID is kept
	for(const auto& p : data)
	{
		i.set(p.first, p.second.get());
	}
}

void unknown::load(storage::InputInterface& i)
{
	base::load(i); // does nothing yet; dunno if it will be needed later
	data = i.copy_all();
}

}
