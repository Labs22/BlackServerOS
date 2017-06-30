
/*
 * libdasm -- simple x86 disassembly library
 * (c) 2004 - 2005  jt / nologin.org
 *
 */


#ifndef _LIBDASM_H
#define _LIBDASM_H

#ifdef __cplusplus
extern "C" {
#endif

// Data types
#if _WIN32
#include <windows.h>
typedef unsigned __int64 QWORD;		// for MSVC
typedef signed   __int8  SBYTE;
typedef signed   __int16 SWORD;
typedef signed   __int32 SDWORD;
typedef signed   __int64 SQWORD;
#define snprintf _snprintf
#else
typedef u_int8_t  BYTE;
typedef u_int16_t WORD;
typedef u_int32_t DWORD;
typedef u_int64_t QWORD;
typedef int8_t    SBYTE;
typedef int16_t   SWORD;
typedef int32_t   SDWORD;
typedef int64_t   SQWORD;
#endif

// Registers
#define REGISTER_EAX 0
#define REGISTER_ECX 1
#define REGISTER_EDX 2
#define REGISTER_EBX 3
#define REGISTER_ESP 4
#define REGISTER_EBP 5
#define REGISTER_ESI 6
#define REGISTER_EDI 7
#define REGISTER_NOP 10	// no register defined

// Register types
#define REGISTER_TYPE_GEN	1
#define REGISTER_TYPE_SEGMENT   2
#define REGISTER_TYPE_DEBUG     3
#define REGISTER_TYPE_CONTROL	4
#define REGISTER_TYPE_TEST      5
#define REGISTER_TYPE_SIMD      6
#define REGISTER_TYPE_MMX       7
#define REGISTER_TYPE_FPU       8

// Disassembling mode
enum Mode {
	MODE_32,	// 32-bit
	MODE_16		// 16-bit
};

// Disassembling format
enum Format {
	FORMAT_ATT,
	FORMAT_INTEL,
};

// Instruction types (just the most common ones atm)
enum Instruction {
	INSTRUCTION_TYPE_MOV,
	INSTRUCTION_TYPE_ADD,
	INSTRUCTION_TYPE_ADC,
	INSTRUCTION_TYPE_SUB,
	INSTRUCTION_TYPE_SBB,
	INSTRUCTION_TYPE_INC,
	INSTRUCTION_TYPE_DEC,
	INSTRUCTION_TYPE_DIV,
	INSTRUCTION_TYPE_NOT,
	INSTRUCTION_TYPE_NEG,
	INSTRUCTION_TYPE_STOS,
	INSTRUCTION_TYPE_LODS,
	INSTRUCTION_TYPE_SCAS,
	INSTRUCTION_TYPE_MOVS,
	INSTRUCTION_TYPE_CMPS,
	INSTRUCTION_TYPE_SHX,	// signed/unsigned shift left/right
	INSTRUCTION_TYPE_ROX,	// signed/unsigned rot left/right
	INSTRUCTION_TYPE_MUL,
	INSTRUCTION_TYPE_XOR,
	INSTRUCTION_TYPE_LEA,
	INSTRUCTION_TYPE_XCHG,
	INSTRUCTION_TYPE_CMP,
	INSTRUCTION_TYPE_TEST,
	INSTRUCTION_TYPE_PUSH,
	INSTRUCTION_TYPE_AND,
	INSTRUCTION_TYPE_OR,
	INSTRUCTION_TYPE_POP,
	INSTRUCTION_TYPE_JMP,
	INSTRUCTION_TYPE_JMPC,	// conditional jump
	INSTRUCTION_TYPE_LOOP,
	INSTRUCTION_TYPE_CALL,
	INSTRUCTION_TYPE_RET,
	INSTRUCTION_TYPE_INT,	// interrupt
	INSTRUCTION_TYPE_FPU,	// FPU-related instruction
	INSTRUCTION_TYPE_OTHER,	// Other instructions :-)
};

// Operand types
enum Operand {
	OPERAND_TYPE_NONE,	// operand not present
	OPERAND_TYPE_MEMORY,	// memory operand ([eax], [0], etc.)
	OPERAND_TYPE_REGISTER,	// register operand (eax, mm0, etc.)
	OPERAND_TYPE_IMMEDIATE,	// immediate operand (0x1234)
};

// Structure definitions

// struct INST is used internally by the library
typedef struct _INST {
	enum Instruction type;	// Instruction type
	const char *mnemonic;	// Instruction mnemonic
	int flags1;		// First operand flags (if any)
	int flags2;		// Second operand flags (if any)
	int flags3;		// Additional operand flags (if any)
	int modrm;		// Is MODRM byte present?
} INST, *PINST;

// Operands for the instruction
typedef struct _OPERAND {
	enum Operand type;	// Operand type (register, memory, etc)
	int reg;		// Register (if any)
	int basereg;		// Base register (if any)
	int indexreg;		// Index register (if any)
	int scale;		// Scale (if any)
	int dispbytes;		// Displacement bytes (0 = no displacement)
	int immbytes;		// Immediate bytes (0 = no immediate)
	int sectionbytes;	// Section prefix bytes (0 = no section prefix)
	WORD section;		// Section prefix value
	DWORD displacement;	// Displacement value
	DWORD immediate;	// Immediate value
	int flags;		// Operand flags
} OPERAND, *POPERAND;

// struct INSTRUCTION is used to interface the library
typedef struct _INSTRUCTION {
	int length;		// Instruction length
	enum Instruction type;	// Instruction type
	enum Mode mode;		// Addressing mode
	BYTE opcode;		// Actual opcode
	BYTE modrm;		// MODRM byte
	BYTE sib;		// SIB byte
	int extindex;		// Extension table index
	int dispbytes;		// Displacement bytes (0 = no displacement)
	int immbytes;		// Immediate bytes (0 = no immediate)
	int sectionbytes;	// Section prefix bytes (0 = no section prefix)
	OPERAND op1;		// First operand (if any)
	OPERAND op2;		// Second operand (if any)
	OPERAND op3;		// Additional operand (if any)
	PINST ptr;		// Pointer to instruction table
	int flags;		// Instruction flags
} INSTRUCTION, *PINSTRUCTION;


// Function definitions

int get_instruction(
	INSTRUCTION *inst,	// pointer to INSTRUCTION structure
	BYTE *addr,		// code buffer
	enum Mode mode		// mode: MODE_32 or MODE_16
);

// Get complete instruction string
int get_instruction_string(
	INSTRUCTION *inst,	// pointer to INSTRUCTION structure
        enum Format format,	// instruction format: FORMAT_ATT or FORMAT_INTEL
	DWORD offset,		// instruction absolute address
	char *string,		// string buffer
	int length		// string length
);

// Get mnemonic string
int get_mnemonic_string(
	INSTRUCTION *inst,	// pointer to INSTRUCTION structure
        enum Format format,	// instruction format: FORMAT_ATT or FORMAT_INTEL
	char *string,		// string buffer
	int length		// string length
);

// Get individual operand string
int get_operand_string(
	INSTRUCTION *inst,	// pointer to INSTRUCTION structure
	POPERAND op,		// pointer to OPERAND structure
        enum Format format,	// instruction format: FORMAT_ATT or FORMAT_INTEL
	DWORD offset,		// instruction absolute address
	char *string,		// string buffer
	int length		// string length
);

// Helper functions

int get_register_type(
	POPERAND op
);
int get_operand_type(
	POPERAND op
);
int get_operand_register(
	POPERAND op
);
int get_operand_basereg(
	POPERAND op
);
int get_operand_indexreg(
	POPERAND op
);
int get_operand_scale(
	POPERAND op
);
int get_operand_immediate(
	POPERAND op,
	DWORD *imm		// returned immediate value
);
int get_operand_displacement(
	POPERAND op,
	DWORD *disp		// returned displacement value
);
POPERAND get_source_operand(
	PINSTRUCTION inst
);
POPERAND get_destination_operand(
	PINSTRUCTION inst
);


// Instruction prefix groups

// Group 1
#define MASK_PREFIX_G1(x) ((x) & 0xFF000000) >> 24
#define PREFIX_LOCK			0x01000000	// 0xf0
#define PREFIX_REPNE			0x02000000	// 0xf2
#define PREFIX_REP			0x03000000	// 0xf3
#define PREFIX_REPE			0x03000000	// 0xf3
// Group 2
#define MASK_PREFIX_G2(x) ((x) & 0x00FF0000) >> 16
#define PREFIX_ES_OVERRIDE		0x00010000	// 0x26
#define PREFIX_CS_OVERRIDE		0x00020000	// 0x2e
#define PREFIX_SS_OVERRIDE		0x00030000	// 0x36
#define PREFIX_DS_OVERRIDE		0x00040000	// 0x3e
#define PREFIX_FS_OVERRIDE		0x00050000	// 0x64
#define PREFIX_GS_OVERRIDE		0x00060000	// 0x65
// Group 3 & 4
#define MASK_PREFIX_G3(x) ((x) & 0x0000FF00) >> 8
#define MASK_PREFIX_OPERAND(x) ((x) & 0x00000F00) >> 8
#define MASK_PREFIX_ADDR(x) ((x) & 0x0000F000) >> 12
#define PREFIX_OPERAND_SIZE_OVERRIDE	0x00000100	// 0x66
#define PREFIX_ADDR_SIZE_OVERRIDE	0x00001000	// 0x67

// Extensions
#define MASK_EXT(x) ((x) & 0x000000FF)
#define EXT_G1 0x00000001
#define EXT_G2 0x00000002
#define EXT_G3 0x00000003
#define EXT_G4 0x00000004
#define EXT_G5 0x00000005
#define EXT_G6 0x00000006
#define EXT_G7 0x00000007
#define EXT_G8 0x00000008
#define EXT_G9 0x00000009
#define EXT_GA 0x0000000a
#define EXT_GB 0x0000000b
#define EXT_GC 0x0000000c
#define EXT_GD 0x0000000d
#define EXT_GE 0x0000000e
#define EXT_GF 0x0000000f
#define EXT_G0 0x00000010	// XXX: what the f***?
#define EXT_T2 0x00000020	// opcode table 2
#define EXT_CP 0x00000030	// co-processor

// Operand flags
#define FLAGS_NONE 0

// Operand Addressing Methods, from the Intel manual
#define MASK_AM(x) ((x) & 0x00FF0000)
#define AM_A 0x00010000		// Direct address with segment prefix
#define AM_C 0x00020000		// MODRM reg field defines control register
#define AM_D 0x00030000		// MODRM reg field defines debug register
#define AM_E 0x00040000		// MODRM byte defines reg/memory address
#define AM_G 0x00050000		// MODRM byte defines general-purpose reg
#define AM_I 0x00060000		// Immediate data follows
#define AM_J 0x00070000		// Immediate value is relative to EIP
#define AM_M 0x00080000		// MODRM mod field can refer only to memory
#define AM_O 0x00090000		// Displacement follows (without modrm/sib)
#define AM_P 0x000a0000		// MODRM reg field defines MMX register
#define AM_Q 0x000b0000		// MODRM defines MMX register or memory 
#define AM_R 0x000c0000		// MODRM mod field can only refer to register
#define AM_S 0x000d0000		// MODRM reg field defines segment register
#define AM_T 0x000e0000		// MODRM reg field defines test register
#define AM_V 0x000f0000		// MODRM reg field defines SIMD register
#define AM_W 0x00100000		// MODRM defines SIMD register or memory 
// Extra addressing modes used in this implementation
#define AM_I1  0x00200000	// Immediate byte 1 encoded in instruction
#define AM_REG 0x00210000	// Register encoded in instruction

// Operand Types, from the intel manual
#define MASK_OT(x) ((x) & 0xFF000000)
#define OT_a  0x01000000
#define OT_b  0x02000000	// always 1 byte
#define OT_c  0x03000000	// byte or word, depending on operand
#define OT_d  0x04000000	// double-word
#define OT_q  0x05000000	// quad-word
#define OT_v  0x06000000	// word or double-word, depending on operand
#define OT_w  0x07000000	// always word
#define OT_ps 0x08000000	// 128-bit packed FP data

// Additional operand flags
#define MASK_FLAGS(x) ((x) & 0x0000FF00)
#define F_s  0x00000100		// sign-extend 1-byte immediate
#define F_r  0x00000200		// use segment register
#define F_f  0x00000300		// use FPU register


#ifdef __cplusplus
}
#endif

#endif
