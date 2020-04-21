#include <hardware_file.h>

#include <iomanip>
#include <sstream>

namespace reven {
namespace vmghost {

hardware_file::hardware_file() : last_read_position_(0), position_(0)
{
}

bool hardware_file::load(const std::string& file_path)
{
	file_.load(file_path);
	position_ = 0;

	if (file_.eof() || !file_.is_open()) {
		return false;
	}

	std::uint64_t magic;
	std::uint32_t version;

	file_ >> magic >> version;

	if (file_.eof()) {
		file_.close();
		return false;
	} else if (magic != HARDWARE_FILE_MAGIC) {
		file_.close();

		std::stringstream error_msg;

		error_msg << "Magic number should be "
		          << std::showbase << std::hex << HARDWARE_FILE_MAGIC
		          << " but is actually "
		          << std::showbase <<  std::hex << magic;

		throw std::runtime_error(error_msg.str());
	}

	last_read_position_ = file_.pos();

	return true;
}

const hardware_access& hardware_file::next()
{
	last_read_position_ = file_.pos();

	if (not file_.eof()) {
		file_ >> current_;
		++position_;
	} else
		current_.tsc = 0;

	return current_;
}

void hardware_file::advance_to(std::uint64_t file_position, std::uint64_t position)
{
	file_.seek(file_position);

	if (position == 0) {
		position_ = 0;
		return;
	}
	position_ = position - 1;
	next();
}

void hardware_file::sync_with(const hardware_file& other)
{
	file_.seek(other.file_.pos());
	position_ = other.position_;

	current_ = other.current_;
}
}
} // namespace reven::vmghost
