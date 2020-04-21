#include <sync_file.h>

#include <iostream>

int main(int argc, char** argv)
{
	reven::vmghost::sync_file file;

	if (argc != 3) {
		std::cerr << "Usage: " << std::endl << argv[0] << " sync_file sync_data_file" << std::endl;
		return 1;
	}

	file.load(argv[1], argv[2]);

	while (file.next().valid()) {
		reven::vmghost::sync_point const& point = file.current();
		std::cout << "TSC=" << point.tsc  << std::endl;
		for(auto const& d: point.data) {
			std::cout << ' ' << d << std::endl;
		}
	}
	return 0;
}
