#include <hardware_file.h>

#include <iostream>

int main(int argc, char** argv)
{
	using namespace reven::vmghost;

	hardware_file file;

	if (argc <= 1) {
		std::cerr << "Usage: " << std::endl << argv[0] << " file [@][trace]" << std::endl;
		return 1;
	}

	file.load(argv[1]);

	if (argc == 2) {
		while (file.next().valid()) {
			std::cout << file.current() << std::endl;
		}
	} else {
		std::string arg = argv[2];
		bool binary_display = false;

		if (arg[0] == '@') {
			binary_display = true;

			arg = arg.substr(1, std::string::npos);
		}
		std::uint64_t num = std::stoul(arg);

		while (file.next().valid() && num > 0)
			--num;

		if (file.next().valid()) {
			if (binary_display) {
				std::cout.write(reinterpret_cast<const char*>(file.current().data.data()), file.current().data.size());
				std::cout << std::flush;
			} else {
				std::cout << file.current() << std::endl;
			}
		}
	}

	return 0;
}
