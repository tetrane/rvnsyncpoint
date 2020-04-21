#include <hardware_access.h>

#include <iomanip>

namespace reven {
namespace vmghost {

std::ostream& operator<<(std::ostream& out, const hardware_access& access)
{
	out << std::showbase << std::hex << std::setfill('0') << "TSC=0x" << access.tsc << ", ";
	if (access.is_pci())
		out << "PCI";
	else if (access.is_mmio())
		out << "MMIO";
	else if (access.is_port())
		out << "Port";
	else
		out << "Unknown type";

	out << " " << (access.is_write() ? "write to " : "read from ") << std::showbase << std::hex << access.physical_address << " = ";

	if (access.data.size() <= 8) {
		out << std::showbase << std::setw(access.data.size() * 2) << access.to_uint64();
	} else if (access.data.size() <= 32) {
		for (std::uint8_t ch : access.data) {
			out << std::setw(2) << static_cast<unsigned>(ch) << " ";
		}
	} else {
		out << "<stripped " << std::dec << access.data.size() << " bytes>";
	}
	out << std::dec;
	return out;
}
}
} // namespace reven::vmghost
