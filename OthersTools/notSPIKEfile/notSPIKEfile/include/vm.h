#ifndef VM_H
#define VM_H
struct REGTBL_ENTRY {
   int size;
   int type;
   void *data;
   char mnemonic[8];
};


/* register types */
#define REG_GENERAL	0x01		/* general-purpose */
#define REG_INPUT		0x02		/* incoming args */
#define REG_OUTPUT	0x04		/* args to subroutines */
#define REG_LOCAL		0x08		/* local vars */
#define REG_FPU		0x10		/* FPU reg */
#define REG_SEG		0x20		/* segment reg */
#define REG_SIMD		0x40		/* simd/mmx stuff */
#define REG_SYS		0x80		/* CPU/OS internal reg */	
#define REG_SP		0x1000	/* stack pointer */
#define REG_FP		0x2000	/* frame pointer */
#define REG_PC		0x4000	/* program counter */
#define REG_RETADD	0x8000	/* return address */
#define REG_CC		0x10000	/* condition code */
#define REG_ZERO		0x20000	/* /dev/null register */
#define REG_RET		0x40000	/* return value */
#define REG_ASRC		0x100000	/* array source */
#define REG_ADEST		0x200000	/* array dest */
#define REG_ACNT		0x400000	/* array length/counter */

#endif
