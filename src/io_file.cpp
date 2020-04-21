#include <io_file.h>

#include <sstream>

namespace reven {
namespace vmghost {

io_file::io_file()
{
}

bool io_file::load(const std::string& file_path)
{
	devices_.clear();

	file_.load(file_path);

	if (file_.eof() || !file_.is_open()) {
		return false;
	}

	std::uint64_t magic = 0;
	file_ >> magic;

	if (file_.eof()) {
		file_.close();
		return false;
	} else if (magic != IO_FILE_MAGIC) {
		file_.close();

		std::stringstream error_msg;

		error_msg << "Magic number should be "
		          << std::showbase << std::hex << IO_FILE_MAGIC
		          << " but is actually "
		          << std::showbase <<  std::hex << magic;

		throw std::runtime_error(error_msg.str());
	}

	std::uint32_t version;
	file_ >> version;

	std::uint64_t size;
	file_ >> size;

	if (file_.eof()) {
		file_.close();
		return false;
	}

	for (std::uint64_t i = 0; i < size; ++i) {
		device d;
		file_ >> d;
		devices_[d.id] = std::move(d);
	}

	return true;
}

}
} // namespace reven::vmghost
