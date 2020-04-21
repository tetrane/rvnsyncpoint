#pragma once

#include "streamable_file.h"
#include "hardware_access.h"

#define HARDWARE_FILE_MAGIC 0x68636e79734e5652

namespace reven {
namespace vmghost {

//! Contains the various hardware related traces.
class hardware_file {
public:
	//! Default constructor
	hardware_file();

	bool load(const std::string& file_name);

	const hardware_access& current() const { return current_; }

	const hardware_access& next();

	void advance_to(std::uint64_t file_position, std::uint64_t position);

	std::uint64_t file_position() const { return last_read_position_; }
	std::uint64_t position() const { return position_; }

	void sync_with(const hardware_file& other);

private:
	//! File that contains the io information.
	streamable_file file_;

	hardware_access current_;

	std::uint64_t last_read_position_;
	std::uint64_t position_;
}; // class hardware_file
}
} // namespace reven::vmghost
