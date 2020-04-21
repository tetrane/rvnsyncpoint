#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

namespace reven {
namespace vmghost {

enum class sync_point_type
{
	VMENTER = 0xff,

	INTERRUPT = 0xfe,

	// These values and names direcly match that of VBox
	VMX_EXIT_XCPT_OR_NMI = 0,
	VMX_EXIT_EXT_INT = 1,
	VMX_EXIT_TRIPLE_FAULT = 2,
	VMX_EXIT_INIT_SIGNAL = 3,
	VMX_EXIT_SIPI = 4,
	VMX_EXIT_IO_SMI = 5,
	VMX_EXIT_SMI = 6,
	VMX_EXIT_INT_WINDOW = 7,
	VMX_EXIT_NMI_WINDOW = 8,
	VMX_EXIT_TASK_SWITCH = 9,
	VMX_EXIT_CPUID = 10,
	VMX_EXIT_GETSEC = 11,
	VMX_EXIT_HLT = 12,
	VMX_EXIT_INVD = 13,
	VMX_EXIT_INVLPG = 14,
	VMX_EXIT_RDPMC = 15,
	VMX_EXIT_RDTSC = 16,
	VMX_EXIT_RSM = 17,
	VMX_EXIT_VMCALL = 18,
	VMX_EXIT_VMCLEAR = 19,
	VMX_EXIT_VMLAUNCH = 20,
	VMX_EXIT_VMPTRLD = 21,
	VMX_EXIT_VMPTRST = 22,
	VMX_EXIT_VMREAD = 23,
	VMX_EXIT_VMRESUME = 24,
	VMX_EXIT_VMWRITE = 25,
	VMX_EXIT_VMXOFF = 26,
	VMX_EXIT_VMXON = 27,
	VMX_EXIT_MOV_CRX = 28,
	VMX_EXIT_MOV_DRX = 29,
	VMX_EXIT_IO_INSTR = 30,
	VMX_EXIT_RDMSR = 31,
	VMX_EXIT_WRMSR = 32,
	VMX_EXIT_ERR_INVALID_GUEST_STATE = 33,
	VMX_EXIT_ERR_MSR_LOAD = 34,
	// value 35 skipped
	VMX_EXIT_MWAIT = 36,
	VMX_EXIT_MTF = 37,
	// value 38 skipped
	VMX_EXIT_MONITOR = 39,
	VMX_EXIT_PAUSE = 40,
	VMX_EXIT_ERR_MACHINE_CHECK = 41,
	// value 42 skipped
	VMX_EXIT_TPR_BELOW_THRESHOLD = 43,
	VMX_EXIT_APIC_ACCESS = 44,
	// value 45 skipped
	VMX_EXIT_XDTR_ACCESS = 46,
	VMX_EXIT_TR_ACCESS = 47,
	VMX_EXIT_EPT_VIOLATION = 48,
	VMX_EXIT_EPT_MISCONFIG = 49,
	VMX_EXIT_INVEPT = 50,
	VMX_EXIT_RDTSCP = 51,
	VMX_EXIT_PREEMPT_TIMER = 52,
	VMX_EXIT_INVVPID = 53,
	VMX_EXIT_WBINVD = 54,
	VMX_EXIT_XSETBV = 55,
	// value 56 skipped
	VMX_EXIT_RDRAND = 57,
	VMX_EXIT_INVPCID = 58,
	VMX_EXIT_VMFUNC = 59
};

const char *sync_point_type_to_string(sync_point_type reason);

enum sync_point_data_type
{
	data_end = 0,

	memory_logical,
	memory_physical,
};

struct sync_point_data
{
	sync_point_data_type type;
	std::uint64_t offset;
	std::vector<std::uint8_t> data;
};

std::ostream& operator<<(std::ostream& out, const sync_point_data& data);

//! Contains a 32 bits synchronization point for the version 1 and 2
//! Used to keep the compatibility with old scenarios
struct sync_point_v2 {
	std::uint32_t eax;
	std::uint32_t ebx;
	std::uint32_t ecx;
	std::uint32_t edx;

	std::uint32_t esi;
	std::uint32_t edi;

	std::uint32_t ebp;
	std::uint32_t esp;

	std::uint32_t eip;
	std::uint32_t eflags;

	std::uint64_t tsc;

	std::uint32_t cr0;
	std::uint32_t cr2;
	std::uint32_t cr3;
	std::uint32_t cr4;
	std::uint16_t fpu_sw;
	std::uint16_t fpu_cw;
	std::uint8_t  fpu_tags;

	std::uint16_t cs;
	std::uint8_t interrupt_vector;
	std::uint16_t fault_error_code;
	sync_point_type type;
}; // class sync_point_v2

//! Contains a synchronization point
struct sync_point_v3 {
	sync_point_v3();
	explicit sync_point_v3(const sync_point_v2&);

	std::uint64_t rax;
	std::uint64_t rbx;
	std::uint64_t rcx;
	std::uint64_t rdx;

	std::uint64_t rsi;
	std::uint64_t rdi;

	std::uint64_t rbp;
	std::uint64_t rsp;

	std::uint64_t r8;
	std::uint64_t r9;
	std::uint64_t r10;
	std::uint64_t r11;
	std::uint64_t r12;
	std::uint64_t r13;
	std::uint64_t r14;
	std::uint64_t r15;

	std::uint64_t rip;
	std::uint64_t rflags;

	std::uint64_t tsc;

	std::uint64_t cr0;
	std::uint64_t cr2;
	std::uint64_t cr3;
	std::uint64_t cr4;
	std::uint16_t fpu_sw;
	std::uint16_t fpu_cw;
	std::uint8_t  fpu_tags;

	std::uint16_t cs;
	std::uint8_t interrupt_vector;
	std::uint32_t fault_error_code;
	sync_point_type type;
	std::vector<sync_point_data> data;

	bool is_interrupt() const { return type == sync_point_type::INTERRUPT; }
	bool is_vmenter() const { return type == sync_point_type::VMENTER; }
	bool is_vmexit() const { return !is_interrupt() && !is_vmenter(); }

	inline bool valid() const { return tsc != 0; }

	bool is_equivalent(const sync_point_v3& rhs) const;
}; // class sync_point_v3

std::ostream& operator<<(std::ostream& out, const sync_point_v3& point);
std::ostream& operator>>(std::ostream& out, const sync_point_v3& point);

bool operator==(const sync_point_v3& lhs, const sync_point_v3& rhs);

using sync_point = sync_point_v3;

}
} // namespace reven::vmghost
