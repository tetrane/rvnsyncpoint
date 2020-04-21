#include <hardware_file.h>
#include <streamable_outfile.h>

#include <iostream>
#include <map>
#include <cassert>

int main(int argc, char** argv)
{
	using namespace reven;
	vmghost::hardware_file file;
	vmghost::streamable_outfile out;

	if (argc != 3) {
		std::cerr << "Usage: " << std::endl << argv[0] << " source dest" << std::endl;
		return 1;
	}

	file.load(argv[1]);
	out.open(argv[2]);

	std::uint64_t magic = HARDWARE_FILE_MAGIC;
	std::uint32_t version = 0;
	out << magic << version;

	std::multimap<std::uint64_t, vmghost::hardware_access> accesses;

	while (file.next().valid()) {
		accesses.insert(std::make_pair(file.current().tsc, file.current()));
	}

	std::uint64_t __attribute__((unused)) current_tsc = 0;
	for (auto& pair : accesses) {
		auto& access = pair.second;
		assert(current_tsc <= access.tsc);

		out << access.tsc << access.physical_address << access.type << access.device_id << access.device_instance
		    << access.data;

		current_tsc = access.tsc;
	}
	return 0;
}
