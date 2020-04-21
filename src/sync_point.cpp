#include <sync_point.h>

#include <ostream>
#include <iomanip>

namespace reven {
namespace vmghost {

std::ostream& operator<<(std::ostream& out, const sync_point_data& data)
{
	out << std::hex;

	switch(data.type) {
		case sync_point_data_type::memory_logical:
			out << "LOGICAL:";
			break;
		case sync_point_data_type::memory_physical:
			out << "PHYSCAL:";
			break;
		default:
			out << "DATAEND:";
			break;
	}

	out << data.offset << '\n';

	int i=0;
	for(auto d: data.data) {
		if(i % 0x10 == 0) {
			if(i) {
				out << '\n';
			}
			out << std::setw(8) << i << ": ";
		}

		out << std::setw(2) << (((unsigned)d) & 0xff) << ' ';
		++i;
	}

	return out;
}

sync_point_v3::sync_point_v3()
    : rax(0),
      rbx(0),
      rcx(0),
      rdx(0),
      rsi(0),
      rdi(0),
      rbp(0),
      rsp(0),
      r8(0),
      r9(0),
      r10(0),
      r11(0),
      r12(0),
      r13(0),
      r14(0),
      r15(0),
      rip(0),
      rflags(0),
      tsc(0),
      cr0(0),
      cr2(0),
      cr3(0),
      cr4(0),
      fpu_sw(0),
      fpu_cw(0),
      fpu_tags(0),
      cs(0),
      interrupt_vector(0),
      fault_error_code(0),
      type(sync_point_type::VMENTER)
{
}

sync_point_v3::sync_point_v3(const sync_point_v2& sp)
	: rax(sp.eax),
      rbx(sp.ebx),
      rcx(sp.ecx),
      rdx(sp.edx),
      rsi(sp.esi),
      rdi(sp.edi),
      rbp(sp.ebp),
      rsp(sp.esp),
      r8(0),
      r9(0),
      r10(0),
      r11(0),
      r12(0),
      r13(0),
      r14(0),
      r15(0),
      rip(sp.eip),
      rflags(sp.eflags),
      tsc(sp.tsc),
      cr0(sp.cr0),
      cr2(sp.cr2),
      cr3(sp.cr3),
      cr4(sp.cr4),
      fpu_sw(sp.fpu_sw),
      fpu_cw(sp.fpu_cw),
      fpu_tags(sp.fpu_tags),
      cs(sp.cs),
      interrupt_vector(sp.interrupt_vector),
      fault_error_code(sp.fault_error_code),
      type(sp.type)
{
}

std::ostream& operator<<(std::ostream& out, const sync_point& point)
{
	out << std::hex;

	if (point.is_interrupt())
		out << std::setfill(' ') << std::setw(45)
			<< "Interrupt 0x" << std::setw(2) << +point.interrupt_vector << std::setfill('0')
			<< "-" << std::setw(8) << point.fault_error_code;
	else
		out << std::setfill(' ') << std::setw(47)
			<< sync_point_type_to_string(point.type) << std::setfill('0');

	out << " RIP=" << std::setw(16) << point.rip
	    << " RFL=" << std::setw(16) << point.rflags
	    << " RAX=" << std::setw(16) << point.rax
	    << " RBX=" << std::setw(16) << point.rbx
	    << " RCX=" << std::setw(16) << point.rcx
	    << " RDX=" << std::setw(16) << point.rdx
	    << " RSI=" << std::setw(16) << point.rsi
	    << " RDI=" << std::setw(16) << point.rdi
	    << " RBP=" << std::setw(16) << point.rbp
	    << " RSP=" << std::setw(16) << point.rsp
	    << " R8=" << std::setw(16) << point.r8
	    << " R9=" << std::setw(16) << point.r9
	    << " R10=" << std::setw(16) << point.r10
	    << " R11=" << std::setw(16) << point.r11
	    << " R12=" << std::setw(16) << point.r12
	    << " R13=" << std::setw(16) << point.r13
	    << " R14=" << std::setw(16) << point.r14
	    << " R15=" << std::setw(16) << point.r15
	    << " CR2=" << std::setw(16) << point.cr2
	    << " CR3=" << std::setw(16) << point.cr3
	    << " FSW=" << std::setw(4) << point.fpu_sw
	    << " FCW=" << std::setw(4) << point.fpu_cw
	    << " FTAGS=" << std::setw(2) << +point.fpu_tags // force 8bit as number
	    << " TSC=" << std::setw(16) << point.tsc;

	out << std::dec;

	return out;
}

bool operator==(const sync_point_v3& lhs, const sync_point_v3& rhs)
{
	return lhs.rax == rhs.rax and lhs.rbx == rhs.rbx and lhs.rcx == rhs.rcx and lhs.rdx == rhs.rdx and
	       lhs.rsi == rhs.rsi and lhs.rdi == rhs.rdi and lhs.rbp == rhs.rbp and lhs.rsp == rhs.rsp and
	       lhs.r8 == rhs.r8 and lhs.r9 == rhs.r9 and lhs.r10 == rhs.r10 and lhs.r11 == rhs.r11 and
	       lhs.r12 == rhs.r12 and lhs.r13 == rhs.r13 and lhs.r14 == rhs.r14 and lhs.r15 == rhs.r15 and
	       lhs.rip == rhs.rip and lhs.rflags == rhs.rflags and lhs.tsc == rhs.tsc and lhs.cr0 == rhs.cr0 and
	       lhs.cr2 == rhs.cr2 and lhs.cr3 == rhs.cr3 and lhs.cr4 == rhs.cr4 and lhs.cs == rhs.cs and
	       lhs.interrupt_vector == rhs.interrupt_vector and lhs.fault_error_code == rhs.fault_error_code and
	       lhs.type == rhs.type;
}

bool sync_point_v3::is_equivalent(const sync_point_v3& rhs) const
{
	return rax == rhs.rax and rbx == rhs.rbx and rcx == rhs.rcx and rdx == rhs.rdx and rsi == rhs.rsi and
	       rdi == rhs.rdi and rbp == rhs.rbp and rsp == rhs.rsp and r8 == rhs.r8 and r9 == rhs.r9 and
	       r10 == rhs.r10 and r11 == rhs.r11 and r12 == rhs.r12 and r13 == rhs.r13 and
	       r14 == rhs.r14 and r15 == rhs.r15 and rip == rhs.rip and rflags == rhs.rflags and
	       cr0 == rhs.cr0 and cr2 == rhs.cr2 and cr3 == rhs.cr3 and cr4 == rhs.cr4 and cs == rhs.cs;
}


const char *sync_point_type_to_string(sync_point_type code)
{
    switch (code) {
	case sync_point_type::VMENTER:
		return "VMenter";
	case sync_point_type::INTERRUPT:
		return "Interrupt";
    case sync_point_type::VMX_EXIT_XCPT_OR_NMI:
		return "Exception or non-maskable interrupt (NMI)";
    case sync_point_type::VMX_EXIT_EXT_INT:
		return "External interrupt";
    case sync_point_type::VMX_EXIT_TRIPLE_FAULT:
		return "Triple fault";
    case sync_point_type::VMX_EXIT_INIT_SIGNAL:
		return "INIT signal";
    case sync_point_type::VMX_EXIT_SIPI:
		return "Start-up IPI (SIPI)";
    case sync_point_type::VMX_EXIT_IO_SMI:
		return "I/O system-management interrupt (SMI)";
    case sync_point_type::VMX_EXIT_SMI:
		return "Other SMI";
    case sync_point_type::VMX_EXIT_INT_WINDOW:
		return "Interrupt window exiting";
    case sync_point_type::VMX_EXIT_NMI_WINDOW:
		return "NMI window exiting";
    case sync_point_type::VMX_EXIT_TASK_SWITCH:
		return "Task switch";
    case sync_point_type::VMX_EXIT_CPUID:
		return "Guest software attempted to execute CPUID";
    case sync_point_type::VMX_EXIT_GETSEC:
		return "Guest software attempted to execute GETSEC";
    case sync_point_type::VMX_EXIT_HLT:
		return "Guest software attempted to execute HLT";
    case sync_point_type::VMX_EXIT_INVD:
		return "Guest software attempted to execute INVD";
    case sync_point_type::VMX_EXIT_INVLPG:
		return "Guest software attempted to execute INVLPG";
    case sync_point_type::VMX_EXIT_RDPMC:
		return "Guest software attempted to execute RDPMC";
    case sync_point_type::VMX_EXIT_RDTSC:
		return "Guest software attempted to execute RDTSC";
    case sync_point_type::VMX_EXIT_RSM:
		return "Guest software attempted to execute RSM in SMM";
    case sync_point_type::VMX_EXIT_VMCALL:
		return "Guest software executed VMCALL";
    case sync_point_type::VMX_EXIT_VMCLEAR:
		return "Guest software executed VMCLEAR";
    case sync_point_type::VMX_EXIT_VMLAUNCH:
		return "Guest software executed VMLAUNCH";
    case sync_point_type::VMX_EXIT_VMPTRLD:
		return "Guest software executed VMPTRLD";
    case sync_point_type::VMX_EXIT_VMPTRST:
		return "Guest software executed VMPTRST";
    case sync_point_type::VMX_EXIT_VMREAD:
		return "Guest software executed VMREAD";
    case sync_point_type::VMX_EXIT_VMRESUME:
		return "Guest software executed VMRESUME";
    case sync_point_type::VMX_EXIT_VMWRITE:
		return "Guest software executed VMWRITE";
    case sync_point_type::VMX_EXIT_VMXOFF:
		return "Guest software executed VMXOFF";
    case sync_point_type::VMX_EXIT_VMXON:
		return "Guest software executed VMXON";
    case sync_point_type::VMX_EXIT_MOV_CRX:
		return "Control-register accesses";
    case sync_point_type::VMX_EXIT_MOV_DRX:
		return "Debug-register accesses";
    case sync_point_type::VMX_EXIT_IO_INSTR:
		return "I/O instruction";
    case sync_point_type::VMX_EXIT_RDMSR: // "RDMSR. Guest software attempted to execute RDMSR";
		return "RDMSR";
    case sync_point_type::VMX_EXIT_WRMSR: // "WRMSR. Guest software attempted to execute WRMSR";
		return "WRMSR";
    case sync_point_type::VMX_EXIT_ERR_INVALID_GUEST_STATE:
		return "VM-entry failure due to invalid guest state";
    case sync_point_type::VMX_EXIT_ERR_MSR_LOAD:
		return "VM-entry failure due to MSR loading";
    case sync_point_type::VMX_EXIT_MWAIT:
		return "Guest software executed MWAIT";
    case sync_point_type::VMX_EXIT_MTF:
		return "VM exit due to monitor trap flag";
    case sync_point_type::VMX_EXIT_MONITOR:
		return "Guest software attempted to execute MONITOR";
    case sync_point_type::VMX_EXIT_PAUSE:
		return "Guest software attempted to execute PAUSE";
    case sync_point_type::VMX_EXIT_ERR_MACHINE_CHECK:
		return "VM-entry failure due to machine-check";
    case sync_point_type::VMX_EXIT_TPR_BELOW_THRESHOLD:
		return "TPR below threshold. Guest software executed MOV to CR8";
    case sync_point_type::VMX_EXIT_APIC_ACCESS: // "APIC access. Guest software attempted to access memory at a physical address on the APIC-access page.";
		return "APIC access";
    case sync_point_type::VMX_EXIT_XDTR_ACCESS: // "Access to GDTR or IDTR. Guest software attempted to execute LGDT, LIDT, SGDT, or SIDT.";
		return "Access to GDTR or IDTR";
    case sync_point_type::VMX_EXIT_TR_ACCESS: // "Access to LDTR or TR. Guest software attempted to execute LLDT, LTR, SLDT, or STR.";
		return "Access to LDTR or TR";
    case sync_point_type::VMX_EXIT_EPT_VIOLATION: // "EPT violation. An attempt to access memory with a guest-physical address was disallowed by the configuration of the EPT paging structures.";
		return "EPT violation";
    case sync_point_type::VMX_EXIT_EPT_MISCONFIG: // "EPT misconfiguration. An attempt to access memory with a guest-physical address encountered a misconfigured EPT paging-structure entry.";
		return "EPT misconfiguration";
    case sync_point_type::VMX_EXIT_INVEPT: // "INVEPT. Guest software attempted to execute INVEPT";
		return "INVEPT";
    case sync_point_type::VMX_EXIT_RDTSCP: // "RDTSCP. Guest software attempted to execute RDTSCP"
		return "RDTSCP";
    case sync_point_type::VMX_EXIT_PREEMPT_TIMER: //"VMX-preemption timer expired. The preemption timer counted down to zero.";
		return "VMX-preemption timer expired";
    case sync_point_type::VMX_EXIT_INVVPID: // "INVVPID. Guest software attempted to execute INVVPID";
		return "INVVPID";
    case sync_point_type::VMX_EXIT_WBINVD: // "WBINVD. Guest software attempted to execute WBINVD";
		return "WBINVD";
    case sync_point_type::VMX_EXIT_XSETBV: // "XSETBV. Guest software attempted to execute XSETBV";
		return "XSETBV";
    case sync_point_type::VMX_EXIT_RDRAND: // "RDRAND. Guest software attempted to execute RDRAND";
		return "RDRAND";
    case sync_point_type::VMX_EXIT_INVPCID: // "INVPCID. Guest software attempted to execute INVPCID"
		return "INVPCID";
    case sync_point_type::VMX_EXIT_VMFUNC: // "VMFUNC. Guest software attempted to execute VMFUNC"
		return "VMFUNC";
    }

    return"Unknown";
}


}
} // namespace reven::vmghost
