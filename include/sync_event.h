#pragma once

#include <cstdint>
#include <ostream>
#include <utility>

#include "sync_point.h"

namespace reven {
namespace vmghost {

struct sync_event
{
	struct context {
		std::uint64_t rax = 0;
		std::uint64_t rbx = 0;
		std::uint64_t rcx = 0;
		std::uint64_t rdx = 0;
		std::uint64_t rsi = 0;
		std::uint64_t rdi = 0;
		std::uint64_t rbp = 0;
		std::uint64_t rsp = 0;
		std::uint64_t r8 = 0;
		std::uint64_t r9 = 0;
		std::uint64_t r10 = 0;
		std::uint64_t r11 = 0;
		std::uint64_t r12 = 0;
		std::uint64_t r13 = 0;
		std::uint64_t r14 = 0;
		std::uint64_t r15 = 0;
		std::uint64_t cr0 = 0;
		std::uint64_t cr2 = 0;
		std::uint64_t cr3 = 0;
		std::uint64_t cr4 = 0;
		std::uint16_t fpu_sw = 0;
		std::uint16_t fpu_cw = 0;
		std::uint8_t  fpu_tags = 0;

		//! The TSC is always part of the context as an input value from VBox, do not check against it.
		std::uint64_t tsc = 0;

		bool are_values_equivalent(const context& other) const; //! Ignores TSC
	};

	//! Position inside the file, in frames
	std::uint64_t position = 0;

	//! An invalid event indicate failure to read it from the file or simply default initialisation, mostly.
	bool is_valid = false;


	//! The first event may lack its eip, but we know must match the first executed instruction, hence we don't need to
	//! check for the start context. Note that if an instruction occurs in that event, we cannot know if we should fire
	//! it during the event or after until the actual Reven execution.
	bool is_first_event_context_unknown = false;

	//! Do not store the EIP in context, because although it is part of the start context, the new_context's EIP is not
	//! relevant as-is (it's meaning changes depending on the event).
	//! is only valid if is_first_event_eip_unknown is false.
	std::uint64_t start_rip = 0;

	//! The machine's context when that event started. The first element of the pair contains whether this element is
	//! known and should be checked.
	context start_context;

	//! The reason this event started (according to VBox)
	sync_point_type start_reason = sync_point_type::VMX_EXIT_XCPT_OR_NMI;

	//! We'll only keep the latest flags.
	//! @TODO: put the flags inside the start_context, and check against them (ignoring the undefined flags)
	std::uint64_t rflags = 0;

	//! The last event's new_context may not be valid, only what's above will be.
	bool is_last_event = false;


	//! Is there an IRQ embedded in this event?
	bool has_interrupt = false;

	//! The interrupt's vector, if any.
	//! Is only valid if has_interrupt is true
	std::uint8_t interrupt_vector = 0;

	//! The EIP the interrupt should be fired at: either current eip or next eip.
	//! Is only valid if has_interrupt is true
	std::uint64_t interrupt_rip = 0;

	//! If the interrupt is a fault, there may be an error code.
	//! Is only valid if has_interrupt is true and the vector is a fault that pushes an error code :
	//! 0x8, 0xa, 0xb, 0xc, 0xd, 0xe, 0x11, 0x1e (according to osdev)
	std::uint32_t fault_error_code = 0;

	//! Does this event actually emulates the current instruction (an indication the real CPU skipped it)?
	//! There may be false positives, ie cases where VBox didn't emulate anything.
	bool is_instruction_emulation = false;

	//! The CPU context at the end of the event. If is_instruction_emulation is true then this context is input from
	//! VBox and we should overrides with these values, otherwise we should check against it.
	context new_context;

	//! Additional dumped data we should check against.
	std::vector<sync_point_data> data;

	//! Will throw an exception if the sanity is compromised
	void check_sanity();
};

std::ostream& operator<<(std::ostream& out, const sync_event& event);

}
} // namespace reven::vmghost
