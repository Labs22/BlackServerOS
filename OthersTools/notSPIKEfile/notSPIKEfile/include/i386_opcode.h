#include "i386.h"

/* Instruction structure -- used for reading opcode table */

typedef struct INSTR {
   int table;                    /* escape to this sub-table */
   int mnemFlg;                 /* Flags referring to mnemonic */
   int destFlg, srcFlg, auxFlg; /* ...and for specific operands */
   int cpu;                       /* minimumCPU [AND with clocks?? */
   char mnemonic[16];           /* buffers for building instruction */
   long dest;//dest[32];
   long src;//src[32];
   long aux;//aux[32];
} instr;

#define MAX_INSTRUCTION_SIZE 20

/* -------------------------------- Opcode Prefixes */
#define INSTR_PREFIX     0xF0000000 /* arbitrary flag # :) */
/* Prefix Flags */
/* Prefixes, same order as in the manual */
#define PREFIX_LOCK       0x00100000
#define PREFIX_REPNZ      0x00200000
#define PREFIX_REPZ       0x00400000
#define PREFIX_REP        0x00800000 
#define PREFIX_REP_SIMD   0x01000000
#define PREFIX_OP_SIZE    0x02000000
#define PREFIX_ADDR_SIZE  0x04000000
#define PREFIX_SIMD       0x08000000
#define PREFIX_CS         0x10000000
#define PREFIX_SS         0x20000000
#define PREFIX_DS         0x30000000
#define PREFIX_ES         0x40000000
#define PREFIX_FS         0x50000000
#define PREFIX_GS         0x60000000
#define PREFIX_REG_MASK   0xF0000000

/* prefix flags table -- used for AND-masking prefixes */
/* these are defined in i386.opcode.map */
extern int prefix_table[13][2];

extern int  reg_gen_type[];

extern char *reg_dword[];
extern char *reg_word[];
extern char *reg_byte[];
extern char *reg_mmx[];
extern char *reg_simd[];
extern char *reg_debug[];
extern char *reg_control[];
extern char *reg_test[];
extern char *reg_seg[];
extern char *reg_fpu[];


/* -------------------------------- Operands */
/* The Flags That Time Forgot */
#define ARG_NONE         0
#define cpu_8086         0x00001000
#define cpu_80286        0x00002000
#define cpu_80386        0x00003000
#define cpu_80387        0x00004000
#define cpu_80486        0x00005000
#define cpu_PENTIUM      0x00006000
#define cpu_PENTPRO      0x00007000
#define cpu_PENTMMX      0x00008000
#define cpu_PENTIUM2     0x00009000
/* Operand classifications, per dislib.h, go to 0x0900 */
#define OPFLAGS_MASK 0x0000FFFF
/* Operand Addressing Methods, per intel manual */
#define   ADDRMETH_MASK      0x00FF0000

#define   ADDRMETH_A  0x00010000   
#define   ADDRMETH_C   0x00020000
#define   ADDRMETH_D   0x00030000
#define   ADDRMETH_E   0x00040000
#define   ADDRMETH_F   0x00050000
#define   ADDRMETH_G   0x00060000
#define   ADDRMETH_I   0x00070000
#define   ADDRMETH_J   0x00080000
#define   ADDRMETH_M   0x00090000
#define   ADDRMETH_O   0x000A0000
#define   ADDRMETH_P   0x000B0000
#define   ADDRMETH_Q   0x000C0000
#define   ADDRMETH_R   0x000D0000
#define   ADDRMETH_S   0x000E0000
#define   ADDRMETH_T   0x000F0000
#define   ADDRMETH_V   0x00100000
#define   ADDRMETH_W   0x00110000
#define   ADDRMETH_X   0x00120000
#define    ADDRMETH_Y   0x00130000

/* Operand Size Codings */
#define OP_SIZE_8      0x00200000
#define OP_SIZE_16   0x00400000
#define OP_SIZE_32   0x00800000

/* Operand Types, per intel manual */
#define OPTYPE_MASK   0xFF000000

#define OPTYPE_a   0x01000000
#define OPTYPE_b   0x02000000
#define OPTYPE_c   0x03000000
#define OPTYPE_d   0x04000000
#define OPTYPE_dq   0x05000000
#define OPTYPE_p   0x06000000
#define OPTYPE_pi   0x07000000
#define OPTYPE_ps   0x08000000
#define OPTYPE_q   0x09000000
#define OPTYPE_s   0x0A000000
#define OPTYPE_ss   0x0B000000
#define OPTYPE_si   0x0C000000
#define OPTYPE_v   0x0D000000
#define OPTYPE_w   0x0E000000
#define OPTYPE_m   0x0F000000	/* to handle LEA */

/* ones I added for FPU instructions */
#define OPTYPE_fs	0x10000000	/* pointer to single-real*/
#define OPTYPE_fd	0x20000000	/* pointer to double real */
#define OPTYPE_fe	0x30000000	/* pointer to extended real */
#define OPTYPE_fb	0x40000000	/* pointer to packed BCD */
#define OPTYPE_fv	0x50000000	/* pointer to FPU env: 14/28-bytes */


/* -------------------------------- ModR/M, SIB */
/* Convenience flags */
#define MODRM_EA  1
#define MODRM_reg 2

/* ModR/M flags */
#define MODRM_RM_SIB         0x04
#define MODRM_RM_NOREG      0x05
/* if (MODRM.MOD_NODISP && MODRM.RM_NOREG) then just disp32 */
#define MODRM_MOD_NODISP   0x00
#define MODRM_MOD_DISP8      0x01
#define MODRM_MOD_DISP32   0x02
#define MODRM_MOD_NOEA      0x03

/* SIB flags */
#define SIB_INDEX_NONE       0x04
#define SIB_BASE_EBP       0x05
#define SIB_SCALE_NOBASE    0x00

/* Convenience struct for modR/M bitfield */
struct modRM_byte {   
   unsigned int mod : 2;
   unsigned int reg : 3;
   unsigned int rm  : 3;
};

/* these are defined in i386.opcode.map */
extern int modrm_rm[];
extern int modrm_reg[];
extern int modrm_mod[];

/* Convenience struct for SIB bitfield */
struct SIB_byte {
   unsigned int scale : 2;
   unsigned int index : 3;
   unsigned int base  : 3;
};

/* these are defined in i386.opcode.map */
extern int sib_scl[];
extern int sib_idx[];
extern int sib_bas[];


/* internal protos */
int GetSizedOperand( unsigned long *op, BYTE *buf, int size);
int DecodeByte(BYTE b, struct modRM_byte *modrm);
int DecodeSIB(BYTE *b, int mod);
int DecodeModRM(BYTE *b, long *op, int *op_flags, int reg_type,
                      int size, int flags);
int InstDecode( instr *t, BYTE *buf, struct code *c, DWORD rva);


/* Convenience struct for opcode tables */
typedef struct x86_table {             //Assembly instruction tables
   instr *table;      //Pointer to table of instruction encodings
   unsigned char shift;              // amount to shift modrm byte
	unsigned char mask;               // bit mask for look up
   unsigned char minlim,maxlim;      // limits on min/max entries.
} asmtable;


/* these are defined in i386.opcode.map */
extern instr tbl_Main[], tbl_0F[], tbl_80[], tbl_81[], tbl_82[], tbl_83[], tbl_C0[];
extern instr tbl_C1[], tbl_D0[], tbl_D1[], tbl_D2[], tbl_D3[], tbl_F6[], tbl_F7[];
extern instr tbl_FE[], tbl_FF[], tbl_0F00[], tbl_0F01[], tbl_0F18[], tbl_0F71[];
extern instr tbl_0F72[], tbl_0F73[], tbl_0FAE[], tbl_0FBA[], tbl_0FC7[];
extern instr tbl_fpuD8_00BF[], tbl_fpuD8_rest[], tbl_fpuD9_00BF[], tbl_fpuD9_rest[];
extern instr tbl_fpuDA_00BF[], tbl_fpuDA_rest[], tbl_fpuDB_00BF[], tbl_fpuDB_rest[];
extern instr tbl_fpuDC_00BF[], tbl_fpuDC_rest[], tbl_fpuDD_00BF[], tbl_fpuDD_rest[];
extern instr tbl_fpuDE_00BF[], tbl_fpuDE_rest[], tbl_fpuDF_00BF[], tbl_fpuDF_rest[];
extern asmtable tables86[];
