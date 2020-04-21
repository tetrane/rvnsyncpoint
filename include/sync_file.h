#pragma once

#include "streamable_file.h"
#include "sync_point.h"
#include "sync_event.h"

namespace reven {
namespace vmghost {

#define SYNC_POINT_MAGIC 0x70636e79734e5652
#define SYNC_POINT_DATA_MAGIC 0x64636e79734e5652
#define SYNC_POINT_FILE_VERSION 3

enum type_flags
{
	irq_mask = 0xff,
	is_irq = 0x100,
};

//! Allows to read the specified sync file.
class sync_file {
public:
	//! Default constructor
	sync_file();

	//! Opens the specified files for input
	sync_file(const std::string& file_name, const std::string& data_file_name);

	//! Opens the specified files for input
	bool load(const std::string& file_name, const std::string& data_file_name);

	//! Retrieves the current synchronization point.
	const sync_point& current() const;

	//! Retrieves the current aggregated synchronization event. May advance the file if necessary
	const sync_event& current_event();

	//! Retrieves the next synchronization point or aggregated event.
	const sync_point& next();

	//! Returns true if there is a scenario file
	bool is_valid() const;

	//! Returns true if there is no more data to read.
	bool eof() const;

	//! Returns the position in number of frames.
	std::uint64_t position() const;

	//! Read frame until we reach the specified position in the file.
	void advance_to(std::uint64_t position);

	//! Read frame until we reach the specified position in the file.
	void seek_from_end(std::uint64_t position);

	void validate(std::uint64_t sequence_id);

	std::uint64_t last_valid_position() const;

	// File capabilities based on version number
	//! The loaded file's version number.
	std::uint32_t version() { return version_; }

	//! True if the loaded file will be missing features
	bool is_deprecated_file() { return version_ < SYNC_POINT_FILE_VERSION; }

	//! Do the sync point contain valid error code for CPU faults?
	bool has_fault_error_codes() { return version_ >= 2; }

	//! Returns the number of sync_point in the file
	std::uint64_t sync_point_count() const { return sync_point_count_; }

private:
	static void point_context_to_event(const sync_point& sp, sync_event::context& context);

	//! Internal seek: will reload current_ but not current_event
	void seek(std::uint64_t position);

	sync_event fetch_new_event();

	//! File that contains the traces
	streamable_file file_;

	//! File that contains the traces
	streamable_file data_file_;

	//! Current sync point
	sync_point current_;

	//! Current sync event
	sync_event current_event_;

	// Position inside the file, in frames
	std::uint64_t position_;

	// Last valid position, i.e. position of the last time we called validate().
	std::uint64_t last_valid_position_;

	// Number of bytes per record. Allows to attempt to load new files if we add more data.
	std::uint32_t record_size_;

	// Version of the file being read
	std::uint32_t version_;

	// The number of sync_point in the file
	std::uint64_t sync_point_count_;

}; // class sync_file
}
} // namespace reven::vmghost
