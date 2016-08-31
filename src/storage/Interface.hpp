#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include <msgpack.hpp>
#include <Poco/DeflatingStream.h>

#include "msgpack_util.hpp"

namespace msgpack {
namespace v1 { // MSGPACK_API_VERSION_NAMESPACE(v1) does not work here?

template<>
template<>
packer<Poco::DeflatingOutputStream>& packer<Poco::DeflatingOutputStream>::pack(const msgpack::sbuffer& v);

} // namespace v1
} // namespace msgpack

namespace Storage {

class InputInterface
{
	public:
		InputInterface(const std::unordered_map<std::string, msgpack::object>& map)
			:
			map(map)
		{
		}

		template<typename T>
		T get(const std::string& key)
		{
			T value;
			find_in_map_or_throw(map, key, value);
			return value;
		}

	private:
		const std::unordered_map<std::string, msgpack::object>& map;
};

class OutputInterface
{
	public:
		template<typename T>
		void set(const std::string& key, T value)
		{
			msgpack::sbuffer buffer;
			msgpack::pack(buffer, value);
			map.emplace(key, std::move(buffer));
		}

		template<typename Stream>
		void flush(msgpack::packer<Stream>& o)
		{
			o.pack(map);
			map.clear();
		}

	private:
		std::unordered_map<std::string, msgpack::sbuffer> map;
};

} // namespace Storage
