#include <sync_file.h>

#include <iostream>

int main(int argc, char** argv)
{
	reven::vmghost::sync_file file;

	if (argc != 2) {
		std::cerr << "Usage: " << std::endl << argv[0] << " file" << std::endl;
		return 1;
	}

	file.load(argv[1], "");

	std::cout << "Number of sync points: " << file.sync_point_count() << std::endl;

	while (file.next().valid()) {
		std::cout << file.current_event() << std::endl;
	}
	return 0;
}
