#include <utility>
#include <ostream>
#include <iomanip>

#include <sync_file.h>
#include <sstream>

#define HEADER_SIZE 1024

namespace reven {
namespace vmghost {

sync_file::sync_file() : position_(0), last_valid_position_(0), sync_point_count_(0)
{
}

sync_file::sync_file(const std::string& file_name, const std::string& data_file_name) : sync_file()
{
	load(file_name, data_file_name);
}

bool sync_file::load(const std::string& file_name, const std::string& data_file_name)
{
	file_.load(file_name);
	position_ = 0;

	if (file_.eof() || !file_.is_open()) {
		return false;
	}

	std::uint64_t magic;
	file_ >> magic;

	if (file_.eof()) {
		file_.close();
		return false;
	} else if (magic != SYNC_POINT_MAGIC) {
		file_.close();
		record_size_ = 0;

		std::stringstream error_msg;

		error_msg << "Magic number should be "
		          << std::showbase << std::hex << SYNC_POINT_MAGIC
		          << " but is actually "
		          << std::showbase <<  std::hex << magic;

		throw std::runtime_error(error_msg.str());
	}

	std::uint32_t vbox_version;

	file_ >> version_ >> vbox_version >> record_size_;

	if (file_.eof()) {
		file_.close();
		return false;
	} else if (version_ > SYNC_POINT_FILE_VERSION) {
		file_.close();
		record_size_ = 0;

		std::stringstream error_msg;

		error_msg << "This version number is not handled: "
		          << std::dec << version_
		          << ", expecting version "
		          << std::dec<< SYNC_POINT_FILE_VERSION;

		throw std::runtime_error(error_msg.str());
	}

	// Calculate the number of sync points in the file
	file_.seek_from_end(0);
	sync_point_count_ = (file_.pos() - HEADER_SIZE) / record_size_;

	// Skip the rest of the header by moving after it.
	file_.seek(HEADER_SIZE);

	if (version_ == 0)
	{
		// No sync data files
		return true;
	}

	data_file_.load(data_file_name);

	if (data_file_.eof()) {
		data_file_.close();
	} else {
		data_file_ >> magic;

		if (magic != SYNC_POINT_DATA_MAGIC) {
			file_.close();
			data_file_.close();

			std::stringstream error_msg;

			error_msg << "Magic number for the data file should be "
			      << std::showbase << std::hex << SYNC_POINT_DATA_MAGIC
			      << " but is actually "
			      << std::showbase <<  std::hex << magic;

			throw std::runtime_error(error_msg.str());
		}
	}

	return true;
}

bool sync_file::is_valid() const
{
	return file_.is_open();
}

bool sync_file::eof() const
{
	return file_.eof();
}

const sync_point& sync_file::current() const
{
	return current_;
}

void sync_file::point_context_to_event(const sync_point& sp, sync_event::context& context)
{
	context.rax = sp.rax;
	context.rbx = sp.rbx;
	context.rcx = sp.rcx;
	context.rdx = sp.rdx;
	context.rsi = sp.rsi;
	context.rdi = sp.rdi;
	context.rbp = sp.rbp;
	context.rsp = sp.rsp;
	context.r8 = sp.r8;
	context.r9 = sp.r9;
	context.r10 = sp.r10;
	context.r11 = sp.r11;
	context.r12 = sp.r12;
	context.r13 = sp.r13;
	context.r14 = sp.r14;
	context.r15 = sp.r15;
	context.cr0 = sp.cr0;
	context.cr2 = sp.cr2;
	context.cr3 = sp.cr3;
	context.cr4 = sp.cr4;
	context.fpu_sw = sp.fpu_sw;
	context.fpu_cw = sp.fpu_cw;
	context.fpu_tags = sp.fpu_tags;
	context.tsc = sp.tsc;
}

const sync_event& sync_file::current_event()
{
	//! This is a bit hacky, but we want to store the current event_
	if (current_event_.is_valid || eof()) {
		return current_event_;
	}

	auto event = fetch_new_event();

	//! Sometimes two events are strictly identical. If:
	//! - two events values are identical and
	//! - the first event is without effect on the scenario (no interrupt, no context change)
	//! -> we'll combine the two to produce only one event.
	//!
	//! This case may turn out problematic, because so far we have no way to be sure these two events are meant to be
	//! matched at the same time, and doing so could cause ordering problems. But one thing is sure: we have seen cases
	//! where these two events had to be matched at the same time so we need this heuristic.
	//!
	//! We ignore position == 1, because it's unlikely we'll see that case on the first instruction, and because the
	//! first case is malformed (no vmexit), it causes seek problems.
	while (event.position != 1 && !event.has_interrupt && event.start_context.are_values_equivalent(event.new_context)) {
		auto event_next = fetch_new_event();
		if (event.new_context.are_values_equivalent(event_next.start_context) &&
		    event.start_rip == event_next.start_rip && !event.is_instruction_emulation) {
			event_next.start_context = event.start_context;
			std::swap(event, event_next);
		} else {
			seek(event_next.position-1);
			break;
		}
	}

	std::swap(current_event_, event);
	return current_event_;
}

sync_event sync_file::fetch_new_event()
{
	sync_event event;

	//! The first VMExit may be missing, in which case we'll rebuild it
	bool first_vmexit_missing = (!current_.is_vmexit() && position_ == 1);

	while (!(current_.is_vmexit() || first_vmexit_missing)) {
		next();

		if(!current_.valid()) {
			return event;
		}
	};

	event.position = position_;

	if (first_vmexit_missing)
	{
		//! Assume the first event _will_ be synced on the first instruction (which makes sense because we activate the
		//! scenario recording during vmexits). There are a few possible cases:
		//!  - The 1st inst. is a in al, dx, which vbox emulates:
		//!     - the vmexits's original eip is vmenter's eip-1
		//!     - this sync point's context contains the new EAX value (because 'in' will overwrite it): we don't know
		//!       the original EAX
		//!  - The 1st inst. is something else not emulated, and EIP is fine.
		//!
		//! -> The consequences are:
		//! we don't know vmexit's EIP, so if there is an IRQ, we don't know whether we should fire it now or later.
		event.is_first_event_context_unknown = true;
		//! Suppose a generic reason for the vmexit
		event.start_reason = sync_point_type::VMX_EXIT_XCPT_OR_NMI;
		//! And we don't really know whether or now there is a context write, assume it is.
		event.is_instruction_emulation = true;
	} else {
		point_context_to_event(current_, event.start_context);
		event.start_rip = current_.rip;
		event.start_reason = current_.type;
		event.data.insert(event.data.end(), current_.data.begin(), current_.data.end());
	}

	if (!first_vmexit_missing) {
		next();

		if(!current_.valid()) {
			event.is_last_event = true;
			event.is_valid = true;
			return event;
		}
	}

	event.data.insert(event.data.end(), current_.data.begin(), current_.data.end());

	//! There may be an interrupt in-between
	if (current_.is_interrupt()) {

		event.has_interrupt = true;
		event.interrupt_vector = current_.interrupt_vector;
		event.interrupt_rip = current_.rip;
		event.fault_error_code = current_.fault_error_code;

		auto interrupt_sp = current_;
		next();

		if(!current_.valid()) {
			event.is_last_event = true;
			event.is_valid = true;
			return event;
		}

		// Sometimes interrupts are reported twice. We should fix that in VBox, but for now, skip it.
		if(current_.is_interrupt()) {
			if (!current_.is_equivalent(interrupt_sp)) {
				std::stringstream error_msg;

				error_msg << "Malformed scenario: context should be identical for duplicated interrupt at $" << std::dec << position_;

				throw std::runtime_error(error_msg.str());
			}

			next();
			if(!current_.valid()) {
				event.is_last_event = true;
				event.is_valid = true;
				return event;
			}
		}

		event.data.insert(event.data.end(), current_.data.begin(), current_.data.end());

		// The TSC might be different and is not checked in is_equivalent
		if (!current_.is_equivalent(interrupt_sp)) {
			std::stringstream error_msg;

			error_msg << "Malformed scenario: context should be identical at $" << std::dec << position_;

			throw std::runtime_error(error_msg.str());
		}
	}

	//! We are now on the last sync point, which will contain the new context. Retrieve it and test
	point_context_to_event(current_, event.new_context);

	//! Eflags may have changed, keep the latest (VMEnter & IRQ would be the same though)
	event.rflags = current_.rflags;

	//! Depending on the exit reason, the new context will contain values to override.
	switch (event.start_reason) {
		case sync_point_type::VMX_EXIT_IO_INSTR:
		case sync_point_type::VMX_EXIT_CPUID:
		case sync_point_type::VMX_EXIT_RDTSC:
		case sync_point_type::VMX_EXIT_RDTSCP:
		case sync_point_type::VMX_EXIT_RDMSR:
		case sync_point_type::VMX_EXIT_MOV_CRX:
		case sync_point_type::VMX_EXIT_EPT_MISCONFIG:
			event.is_instruction_emulation = true;
			break;
		case sync_point_type::VMX_EXIT_HLT:
			// Do nothing, we'll skip the instruction
			break;
		case sync_point_type::VMX_EXIT_XCPT_OR_NMI:
			//! VBox emulated something, but we have no idea what it did. If EIP changes, assume it is an emulation.
			if (!event.is_first_event_context_unknown && current_.rip != event.start_rip) {
				event.is_instruction_emulation = true;
			}
			break;
		default:
			//! For now the rest isn't supposed to do anything. Add cases along the way.
			if (!event.is_first_event_context_unknown && current_.rip != event.start_rip) {
				event.is_instruction_emulation = true;
			}
			break;
	}


	if (!current_.is_vmenter()) {
		std::stringstream error_msg;

		error_msg << "Malformed scenario: VMExit without a VMEnter at $" << std::dec << position_;

		throw std::runtime_error(error_msg.str());
	}

	event.check_sanity();

	event.is_valid = true;
	return event;
}

const sync_point& sync_file::next()
{
	std::uint16_t type;
	std::uint32_t data_offset;
	std::uint8_t padding;

	if (eof()) {
		return current_;
	}

	auto prev = current_;

	do {
		if (version_ < 3) {
			sync_point_v2 old_sp;

			file_ >> old_sp.tsc >> type >> old_sp.cs
			  >> old_sp.eax >> old_sp.ebx >> old_sp.ecx >> old_sp.edx
			  >> old_sp.esi >> old_sp.edi
			  >> old_sp.ebp >> old_sp.esp
			  >> old_sp.eip >> old_sp.eflags
			  >> old_sp.cr0 >> old_sp.cr2 >> old_sp.cr3 >> old_sp.cr4
			  >> data_offset >> old_sp.fpu_sw >> old_sp.fpu_cw >> old_sp.fpu_tags
			  >> old_sp.fault_error_code;

			file_ >> padding;

			current_ = sync_point(old_sp);
		} else {
			file_ >> current_.tsc >> type >> current_.cs
			  >> current_.rax >> current_.rbx >> current_.rcx >> current_.rdx
			  >> current_.rsi >> current_.rdi
			  >> current_.rbp >> current_.rsp
			  >> current_.r8 >> current_.r9 >> current_.r10 >> current_.r11
			  >> current_.r12 >> current_.r13 >> current_.r14 >> current_.r15
			  >> current_.rip >> current_.rflags
			  >> current_.cr0 >> current_.cr2 >> current_.cr3 >> current_.cr4
			  >> data_offset >> current_.fpu_sw >> current_.fpu_cw >> current_.fpu_tags
			  >> current_.fault_error_code;

			// We are reading 201 bytes from the file.
			// In the version 3, record size is 256 so we need to also read the padding before reading the next entry.
			for (unsigned i = 0; i < record_size_ - 201; ++i)
				file_ >> padding;
		}

		bool is_irq = (type & type_flags::is_irq) != 0;

		if (is_irq) {
			current_.type = sync_point_type::INTERRUPT;
			current_.interrupt_vector = (type & type_flags::irq_mask);
		} else {
			current_.type = static_cast<sync_point_type>(type & type_flags::irq_mask);
		}

		current_.data.clear();

		if (data_offset && !data_file_.eof()) {
			data_file_.seek(data_offset);

			while (true) {
				std::uint32_t data_type;
				sync_point_data d;

				data_file_ >> data_type;

				d.type = static_cast<sync_point_data_type>(data_type);

				if (d.type == sync_point_data_type::data_end)
					break;

				data_file_ >> d.offset >> d.data;

				current_.data.push_back(d);
			}
		}

		if (not eof()) {
			++position_;
		}
	} while (prev == current_ && not eof()); //! Skip identical sync points

	if (current_event_.is_valid)
		current_event_ = sync_event();

	return current_;
}

std::uint64_t sync_file::position() const
{
	return position_;
}

void sync_file::advance_to(std::uint64_t position)
{
	seek(position);

	if (position == 0)
		return;

	if (current_.is_vmenter()) {
		// Hack: reload the current event.
		while (!current_.is_vmexit() && position_ > 1) {
			position = position - 1;
			seek(position);
		}
		current_event();
	}
}

void sync_file::seek(std::uint64_t position)
{
	if (!record_size_) {
		position_ = 0;
		return;
	}

	if (position == 0) {
		last_valid_position_ = position;
		current_ = sync_point();
		file_.seek(HEADER_SIZE);
		position_ = 0;
		return;
	}

	position_ = position - 1;
	file_.seek(HEADER_SIZE + record_size_ * position_);
	current_ = sync_point();
	next();

	last_valid_position_ = position_ == 0 ? 0 : position_ - 1;
}

void sync_file::seek_from_end(std::uint64_t position)
{
	if (!record_size_) {
		position_ = 0;
		return;
	}
	current_ = sync_point();
	file_.seek_from_end(record_size_ * (position + 1));
	position_ = (file_.pos() - HEADER_SIZE) / record_size_;

	last_valid_position_ = position;
}

void sync_file::validate(std::uint64_t __attribute__((unused)) sequence_id)
{
	if (last_valid_position_ != position()) {
		last_valid_position_ = position();
	}
}

std::uint64_t sync_file::last_valid_position() const
{
	return last_valid_position_;
}
}
} // namespace reven::vmghost
