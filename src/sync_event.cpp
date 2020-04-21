#include <sync_point.h>
#include <sync_event.h>

#include <ostream>
#include <iomanip>

namespace reven {
namespace vmghost {

std::ostream& operator<<(std::ostream& out, const sync_event& event)
{
	if (!event.is_valid) {
		out << "Invalid event." << std::endl;
		return out;
	}
	out << "Sync Event pos $" << event.position << " - " << sync_point_type_to_string(event.start_reason) << std::endl;

	out << std::hex;

	out << std::setfill('0');
	if (!event.is_first_event_context_unknown) {
		out << "RIP=" << std::setw(16) << event.start_rip
		    << " RFL=" << std::setw(16) << event.rflags // 22 useful bits only
		    << " RAX=" << std::setw(16) << event.start_context.rax
		    << " RBX=" << std::setw(16) << event.start_context.rbx
		    << " RCX=" << std::setw(16) << event.start_context.rcx
		    << " RDX=" << std::setw(16) << event.start_context.rdx
		    << " RSI=" << std::setw(16) << event.start_context.rsi
		    << " RDI=" << std::setw(16) << event.start_context.rdi
		    << " RBP=" << std::setw(16) << event.start_context.rbp
		    << " RSP=" << std::setw(16) << event.start_context.rsp
		    << " R8=" << std::setw(16) << event.start_context.r8
		    << " R9=" << std::setw(16) << event.start_context.r9
		    << " R10=" << std::setw(16) << event.start_context.r10
		    << " R11=" << std::setw(16) << event.start_context.r11
		    << " R12=" << std::setw(16) << event.start_context.r12
		    << " R13=" << std::setw(16) << event.start_context.r13
		    << " R14=" << std::setw(16) << event.start_context.r14
		    << " R15=" << std::setw(16) << event.start_context.r15
		    << " CR0=" << std::setw(16) << event.start_context.cr0
		    << " CR2=" << std::setw(16) << event.start_context.cr2
		    << " CR3=" << std::setw(16) << event.start_context.cr3
		    << " CR4=" << std::setw(16) << event.start_context.cr4
		    << " FSW=" << std::setw(4) << event.start_context.fpu_sw
		    << " FCW=" << std::setw(4) << event.start_context.fpu_cw
		    << " FTAGS=" << std::setw(2) << +event.start_context.fpu_tags // force 8bit as number
		    << " TSC=" << std::setw(16) << event.start_context.tsc;
	} else {
		out << "First event with unknown VMExit: no input context";
	}

	out << std::endl;

	if (event.has_interrupt) {
		out << "Interrupt 0x" << std::setw(2) << (std::uint32_t)event.interrupt_vector << std::setfill('0');
		if (event.is_first_event_context_unknown || event.interrupt_rip != event.start_rip)
			out << " at " << std::setw(16) << event.interrupt_rip;
		else
			out << " now";
		out << " with code " << std::setw(8) << event.fault_error_code;

		out << std::endl;
	}

	if (event.is_last_event) {
		out << "Is last!";
	} else {
		out << "New context " << (event.is_instruction_emulation ? "emulated" : "to check") << " :";

#define COMPARE_AND_DISPLAY(reg, name, size)                                                                                          \
	if (event.is_first_event_context_unknown || event.new_context.reg != event.start_context.reg)                      \
		out << " " name "=" << std::setw(size) << +event.new_context.reg;

		COMPARE_AND_DISPLAY(rax, "RAX", 16);
		COMPARE_AND_DISPLAY(rbx, "RBX", 16);
		COMPARE_AND_DISPLAY(rcx, "RCX", 16);
		COMPARE_AND_DISPLAY(rdx, "RDX", 16);
		COMPARE_AND_DISPLAY(rsi, "RSI", 16);
		COMPARE_AND_DISPLAY(rdi, "RDI", 16);
		COMPARE_AND_DISPLAY(rbp, "RBP", 16);
		COMPARE_AND_DISPLAY(rsp, "RSP", 16);
		COMPARE_AND_DISPLAY(r8, "R8", 16);
		COMPARE_AND_DISPLAY(r9, "R9", 16);
		COMPARE_AND_DISPLAY(r10, "R10", 16);
		COMPARE_AND_DISPLAY(r11, "R11", 16);
		COMPARE_AND_DISPLAY(r12, "R12", 16);
		COMPARE_AND_DISPLAY(r13, "R13", 16);
		COMPARE_AND_DISPLAY(r14, "R14", 16);
		COMPARE_AND_DISPLAY(r15, "R15", 16);
		COMPARE_AND_DISPLAY(cr0, "CR0", 16);
		COMPARE_AND_DISPLAY(cr2, "CR2", 16);
		COMPARE_AND_DISPLAY(cr3, "CR3", 16);
		COMPARE_AND_DISPLAY(cr4, "CR4", 16);
		COMPARE_AND_DISPLAY(fpu_sw, "FSW", 4);
		COMPARE_AND_DISPLAY(fpu_cw, "FCW", 4);
		COMPARE_AND_DISPLAY(fpu_tags, "FTAGS", 2);
		COMPARE_AND_DISPLAY(tsc, "TSC", 16);
	}

	out << std::endl;

	out << std::dec;

	return out;
}

bool sync_event::context::are_values_equivalent(const context& other) const
{
	return rax == other.rax && rbx == other.rbx && rcx == other.rcx && rdx == other.rdx && rsi == other.rsi &&
	       rdi == other.rdi && rbp == other.rbp && rsp == other.rsp && r8 == other.r8 && r9 == other.r9 &&
	       r10 == other.r10 && r11 == other.r11 && r12 == other.r12 && r13 == other.r13 && r14 == other.r14 &&
	       r15 == other.r15 && cr0 == other.cr0 && cr2 == other.cr2 && cr3 == other.cr3 && cr4 == other.cr4;
}

#ifdef RVN_MACHINES_DEBUG_SYNC
#define CHECK_REG_VALIDITY(reg)                                                                                        \
	if (start_context.reg != new_context.reg) {                                                                        \
		throw std::runtime_error("Unforseen context change in reg");                                                   \
	}
#else
#define CHECK_REG_VALIDITY(reg)
#endif

void sync_event::check_sanity()
{
	if (!is_first_event_context_unknown && has_interrupt) {
		if (is_instruction_emulation && start_rip == interrupt_rip) {
			throw std::runtime_error("Instruction emulation but immediate interrupt combined: not handled");
		}
		if (new_context.cr2 != start_context.cr2 && interrupt_vector != 0x0e) {
			throw std::runtime_error("CR2 change with non page-fault interrupt: not supported");
		}
		if (interrupt_vector < 0x20 && start_rip != interrupt_rip) {
			throw std::runtime_error("CPU Fault that should wait doesn't make sense");
		}
	}
	if (!is_instruction_emulation) {
		CHECK_REG_VALIDITY(rax);
		CHECK_REG_VALIDITY(rbx);
		CHECK_REG_VALIDITY(rcx);
		CHECK_REG_VALIDITY(rdx);
		CHECK_REG_VALIDITY(rsi);
		CHECK_REG_VALIDITY(rdi);
		CHECK_REG_VALIDITY(rbp);
		CHECK_REG_VALIDITY(rsp);
		CHECK_REG_VALIDITY(r8);
		CHECK_REG_VALIDITY(r9);
		CHECK_REG_VALIDITY(r10);
		CHECK_REG_VALIDITY(r11);
		CHECK_REG_VALIDITY(r12);
		CHECK_REG_VALIDITY(r13);
		CHECK_REG_VALIDITY(r14);
		CHECK_REG_VALIDITY(r15);
		CHECK_REG_VALIDITY(rip);
		CHECK_REG_VALIDITY(cr0);
		CHECK_REG_VALIDITY(cr2);
		CHECK_REG_VALIDITY(cr3);
	}
}


}
} // namespace reven::vmghost
