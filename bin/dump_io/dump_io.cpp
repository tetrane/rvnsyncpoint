#include <io_file.h>

#include <iostream>

int main(int argc, char** argv)
{
	using namespace reven::vmghost;

	io_file file;

	if (argc <= 1) {
		std::cerr << "Usage: " << std::endl << argv[0] << " file" << std::endl;
		return 1;
	} else {
		file.load(argv[1]);
	}

	std::cout << std::hex << std::showbase;

	for (const auto& pair : file.devices()) {
		const auto& dev = pair.second;
		std::cout << dev.name << " - " << dev.description << "Id: " << dev.id << std::endl;
		if (not dev.port_ranges.empty()) {
			std::cout << "Port mappings:" << std::endl;
			for (const port_range& port : dev.port_ranges) {
				std::cout << "    " << port.port << "-" << (port.port + port.length) << ": " << port.description
				          << std::endl;
			}
		}
		if (not dev.memory_ranges.empty()) {
			std::cout << "Memory mappings:" << std::endl;
			for (const memory_range& mem : dev.memory_ranges) {
				std::cout << "    " << mem.physical_address << "-" << (mem.physical_address + mem.length) << ": "
				          << mem.description << std::endl;
			}
		}
		std::cout << std::endl;
	}

	return 0;
}
