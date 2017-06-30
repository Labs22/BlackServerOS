#ifndef BASTARD_ARCH_I386_H
#define BASTARD_ARCH_I386_H

#include <bastard.h>
#include <extension.h>
#include <vm.h>
#include <dirent.h>

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;

/* opcode tables */
#define  x86_MAIN 0
#define  x86_0F   1
#define  x86_80   2

#define REG_DWORD_OFFSET 0
#define REG_WORD_OFFSET 1 * 8
#define REG_BYTE_OFFSET 2 * 8
#define REG_MMX_OFFSET 3 * 8
#define REG_SIMD_OFFSET 4 * 8
#define REG_DEBUG_OFFSET 5 * 8
#define REG_CTRL_OFFSET 6 * 8
#define REG_TEST_OFFSET 7 * 8
#define REG_SEG_OFFSET 8 * 8
#define REG_FPU_OFFSET 9 * 8
#define REG_FLAGS_INDEX 10 * 8
#define REG_FPCTRL_INDEX 10 * 8 + 1
#define REG_FPSTATUS_INDEX 10 * 8 + 2
#define REG_FPTAG_INDEX 10 * 8 + 3
#define REG_EIP_INDEX 10 * 8 + 4
#define REG_IP_INDEX 10 * 8 + 5

#define REG_DWORD_SIZE 4
#define REG_WORD_SIZE 2
#define REG_BYTE_SIZE 1
#define REG_MMX_SIZE 4
#define REG_SIMD_SIZE 4
#define REG_DEBUG_SIZE 4
#define REG_CTRL_SIZE 4
#define REG_TEST_SIZE 4
#define REG_SEG_SIZE 2
#define REG_FPU_SIZE 10
#define REG_FLAGS_SIZE 4
#define REG_FPCTRL_SIZE 2
#define REG_FPSTATUS_SIZE 2
#define REG_FPTAG_SIZE 2
#define REG_EIP_SIZE 4
#define REG_IP_SIZE 2
/* add TR LDTR [6 bytes] and IDTR GDTR (4bytes)


/* prototypes */
void ext_arch_init( void *param);
void InitRegTable( void );
int get_prologue(struct code **table);
int get_epilogue(struct code **table);
int disasm_addr( BYTE *buf, int tbl, struct code *c, long rva);
int disasm_invariant( unsigned char *in, int in_len, unsigned char *out, int out_len );


/* Module options */
/* these two are now defined in arch.h */
//#define IGNORE_NULLS    0x01  /* don't disassemble sequences of > 4 NULLs */
//#define MODE_16_BIT     0x02  /* use useless 16bit mode */
/* the following dictate what ISAs to support */
/* for now these do nothing */
#define ISA_8086        0x10
#define ISA_80286       0x20
#define ISA_80386       0x40
#define ISA_80486       0x80
#define ISA_PENTIUM     0x100
#define ISA_PENTIUM_2   0x200  
#define ISA_PENTIUM_3   0x400
#define ISA_PENTIUM_4   0x800
#define ISA_K6          0x1000
#define ISA_K7          0x2000
#define ISA_ATHLON      0x4000
#define ISA_SIMD        0x10000
#define ISA_MMX         0x20000
#define ISA_3DNOW       0x40000

#endif
