#include <streamable_file.h>

namespace reven {
namespace vmghost {

streamable_file::streamable_file() : eof_(true)
{
}

void streamable_file::load(const std::string& file_name)
{
	in_file_.close();
	eof_ = false;

	in_file_.open(file_name, std::ifstream::binary | std::ifstream::in);

	opened_ = in_file_.is_open();

	check_eof();
}

void streamable_file::close()
{
	in_file_.close();
	eof_ = true;
	opened_ = false;
}
}
} // namespace reven::vmghost
