#pragma once

#include "streamable_file.h"

namespace reven {
namespace vmghost {

struct port_range {
	std::string description;
	std::uint16_t port;
	std::uint16_t length;
	std::uint32_t instance;
};

inline streamable_file& operator>>(streamable_file& in, port_range& port)
{
	in >> port.description >> port.port >> port.length >> port.instance;
	return in;
}

struct memory_range {
	std::string description;
	std::uint64_t physical_address;
	std::uint64_t length;
	std::uint32_t instance;
};

inline streamable_file& operator>>(streamable_file& in, memory_range& mmio)
{
	in >> mmio.description >> mmio.physical_address >> mmio.length >> mmio.instance;
	return in;
}

struct device {
	std::string name;
	std::string description;
	std::uint64_t id;
	std::vector<port_range> port_ranges;
	std::vector<memory_range> memory_ranges;
};

inline streamable_file& operator>>(streamable_file& in, device& dev)
{
	in >> dev.name >> dev.description >> dev.id >> dev.port_ranges >> dev.memory_ranges;
	return in;
}

}} // namespace reven::vmghost
