#pragma once

#include "streamable_file.h"

namespace reven {
namespace vmghost {

//! Represents an atomic hardware access to the main memory.
//!
//! The hardware accesses are recorded as is during the scenario, to be replayed when the conditions of the scenario
//! match with the emulated conditions.
struct hardware_access {
	enum hardware_access_type {
		write = 0x000000001,
		pci = 0x000000002,
		mmio = 0x000000004,
		port = 0x000000008,
	};

	//! Timestamp counter when the hardware access occurred.
	//!
	//! We will probably have a corresponding sync_file entry not far from this one, but don't count on it.
	//! For example, using MSI (Message Signaled Interrupts), only some part of main memory is written, no actual
	//! interrupt handler is called. This means that there is not necessarily a VMexit when memory is written, and
	//! that we need to guess when the write actually took place.
	std::uint64_t tsc = 0;

	//! Physical address in the main memory (RAM) that the hardware accessed.
	std::uint64_t physical_address;

	//! Type of access. This is a bitfield oh hardware_access_type elements, use of the hepler functions is recommended.
	std::uint64_t type;

	//! Identifier of the device requesting the information. 0 if unknown.
	std::uint64_t device_id;

	//! Instance of the device when several instances of the same device exist.
	std::uint32_t device_instance;

	//! Data concerned by the read or write.
	//! If this is a write, this is the content that the hardware set into memory.
	//! If this is a read, this is the content that the hardware received in the recorded scenario.
	std::vector<std::uint8_t> data;

	//! Number of bytes read or written by the hardware.
	std::uint64_t length() const { return data.size(); }

	//! Returns true if the specified flag is set in the type variable.
	bool has_type(hardware_access_type checked_type) const { return (type & checked_type) == checked_type; }

	//! True for Memory Mapped IO accesses.
	//! MMIO is a bit special because the read and write semantics are kind of reversed:
	//!
	//! If the CPU does MOV EAX, [EDX] while EDX is a pointer to a MMIO register,
	//! we're actually reading from memory.
	//!
	//! But on the hardware side, we are writing something. On Reven, we emulate this by making the
	//! hardware write into memory just before we actually read from it.
	bool is_mmio() const { return has_type(hardware_access_type::mmio); }

	//! True for PCI accesses. The data buffers can be a bit big on those access types.
	bool is_pci() const { return has_type(hardware_access_type::pci); }

	//! True for port accesses.
	bool is_port() const { return has_type(hardware_access_type::port); }

	//! True if the hardware is writing into main memory.
	bool is_write() const { return has_type(hardware_access_type::write); }

	//! Convert the first high order bytes of the data into a number.
	//!
	//! This is mostly useful to be able to retrieve small data elements.
	std::uint64_t to_uint64() const
	{
		std::uint64_t result = 0;

		::memcpy(&result, data.data(), std::min(data.size(), sizeof(std::uint64_t)));

		return result;
	}

	bool valid() const { return tsc != 0; }
};

std::ostream& operator<<(std::ostream& out, const hardware_access& access);

inline streamable_file& operator>>(streamable_file& in, hardware_access& access)
{
	in >> access.tsc >> access.physical_address >> access.type >> access.device_id >> access.device_instance >>
	    access.data;
	return in;
}
}
} // namespace reven::vmghost
