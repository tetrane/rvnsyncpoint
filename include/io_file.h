#pragma once

#include "streamable_file.h"
#include "device.h"

#include <map>

#define IO_FILE_MAGIC 0x69636e79734e5652

namespace reven {
namespace vmghost {

//! Contains the various io related traces.
class io_file {
public:
	//! Default constructor
	io_file();

	bool load(const std::string& file_name);

	const std::map<std::uint64_t, device>& devices() const { return devices_; }

private:
	//! File that contains the io information.
	streamable_file file_;

	std::map<std::uint64_t, device> devices_;
}; // class io_file
}
} // namespace reven::vmghost
