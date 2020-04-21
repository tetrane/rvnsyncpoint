#include <streamable_outfile.h>

namespace reven {
namespace vmghost {

streamable_outfile::streamable_outfile() = default;

void streamable_outfile::open(const std::string& file_name)
{
	out_file_.close();
	out_file_.open(file_name, std::ofstream::binary | std::ofstream::out);
}

void streamable_outfile::close()
{
	out_file_.close();
}
}
} // namespace reven::vmghost
