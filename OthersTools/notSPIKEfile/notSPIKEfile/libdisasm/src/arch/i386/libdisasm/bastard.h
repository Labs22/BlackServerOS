#ifndef BASTARD_H
#define BASTARD_H
/* stripped-down combination bastard.h and bdb.h for libdisasm */
struct addr_exp {  
    int     scale;
    int     index;
    int     base;
    int     disp;
    int     flags;
    int     used;
};

struct code { 
    long rva;
    long func; /* unused in libdisasm */
    char    mnemonic[16];
    long    dest;
    long    src;
    long    aux;
    unsigned int     mnemType;
    unsigned int     destType;
    unsigned int     srcType;
    unsigned int     auxType;
    unsigned long    destConst;
    unsigned long    srcConst;
    unsigned long    auxConst;
};
struct code_effect {  /* size 16 */
    unsigned long    id;
    unsigned long    rva;
    int     reg;
    int     change;
};

/* copied from extensions/arch.h in the bastard */
/* Public Parts */
#define BIG_ENDIAN_ORD       0
#define LITTLE_ENDIAN_ORD    1

/* disassembler options */
#define IGNORE_NULLS    0x01  /* don't disassemble sequences of > 4 NULLs */
#define LEGACY_MODE     0x02  /* e.g. 16-bit on Intel */


 /* Operand and instruction types */
/*                   Permissions: */
#define OP_R         0x001      /* operand is READ */
#define OP_W         0x002      /* operand is WRITTEN */
#define OP_X         0x004      /* operand is EXECUTED */
/*                   Types: */
#define OP_UNK       0x000      /* unknown operand */     
#define OP_REG       0x100      /* register */
#define OP_IMM       0x200      /* immediate value */
#define OP_REL       0x300      /* relative Address [offset from IP] */
#define OP_ADDR      0x400      /* Absolute Address */
#define OP_EXPR      0x500      /* Address Expression [e.g. SIB byte] */
#define OP_PTR       0x600      /* Operand is an Address containing a Pointer */
#define OP_OFF       0x700      /* Operand is an offset from a seg/selector */
/*                   Modifiers: */
#define OP_SIGNED    0x001000   /* operand is signed */
#define OP_STRING    0x002000   /* operand a string */
#define OP_CONST     0x004000   /* operand is a constant */
#define OP_EXTRASEG  0x010000   /* seg overrides */
#define OP_CODESEG   0x020000
#define OP_STACKSEG  0x030000
#define OP_DATASEG   0x040000
#define OP_DATA1SEG  0x050000
#define OP_DATA2SEG  0x060000
/*                   Size: */
#define OP_BYTE      0x100000   /* operand is 8 bits/1 byte  */
#define OP_HWORD     0x200000   /* operand is .5 mach word (Intel 16 bits) */
#define OP_WORD      0x300000   /* operand is 1 machine word (Intel 32 bits) */
#define OP_DWORD     0x400000   /* operand is 2 mach words (Intel 64 bits) */
#define OP_QWORD     0x500000   /* operand is 4 mach words (Intel 128 bits) */
#define OP_SREAL     0x600000   /* operand is 32 bits/4 bytes */
#define OP_DREAL     0x700000   /* operand is 64 bits/8 bytes */
#define OP_XREAL     0x800000   /* operand is 40 bits/10 bytes */
#define OP_BCD       0x900000   /* operand is 40 bits/10 bytes */            
#define OP_SIMD      0xA00000   /* operand is 128 bits/16 bytes */            
#define OP_FPENV     0xB00000   /* operand is 224 bits/28 bytes */            

/* operand masks */
#define OP_PERM_MASK 0x0000007  /* perms are NOT mutually exclusive */
#define OP_TYPE_MASK 0x0000F00  /* types are mututally exclusive */
#define OP_MOD_MASK  0x00FF000  /* mods are NOT mutual;y exclusive */
#define OP_SEG_MASK  0x00F0000  /* segs are NOT mutually exclusive */
#define OP_SIZE_MASK 0x0F00000  /* sizes are mutually exclusive */

#define OP_REG_MASK    0x0000FFFF /* lower WORD is register ID */
#define OP_REGTBL_MASK 0xFFFF0000 /* higher word is register type [gen/dbg] */

/* instruction types [groups] */
#define INS_EXEC		0x1000
#define INS_ARITH		0x2000
#define INS_LOGIC		0x3000
#define INS_STACK		0x4000
#define INS_COND		0x5000
#define INS_LOAD		0x6000
#define INS_ARRAY		0x7000
#define INS_BIT		0x8000
#define INS_FLAG		0x9000
#define INS_FPU		0xA000
#define INS_TRAPS		0xD000
#define INS_SYSTEM	0xE000
#define INS_OTHER		0xF000

/* INS_EXEC group */
#define INS_BRANCH	INS_EXEC | 0x01	/* Unconditional branch */
#define INS_BRANCHCC	INS_EXEC | 0x02	/* Conditional branch */
#define INS_CALL		INS_EXEC | 0x03	/* Jump to subroutine */
#define INS_CALLCC	INS_EXEC | 0x04	/* Jump to subroutine */
#define INS_RET		INS_EXEC | 0x05	/* Return from subroutine */
#define INS_LOOP		INS_EXEC | 0x06	/* loop to local label */

/* INS_ARITH group */
#define INS_ADD 		INS_ARITH | 0x01
#define INS_SUB		INS_ARITH | 0x02
#define INS_MUL		INS_ARITH | 0x03
#define INS_DIV		INS_ARITH | 0x04
#define INS_INC		INS_ARITH | 0x05	/* increment */
#define INS_DEC		INS_ARITH | 0x06	/* decrement */
#define INS_SHL		INS_ARITH | 0x07	/* shift right */
#define INS_SHR		INS_ARITH | 0x08	/* shift left */
#define INS_ROL		INS_ARITH | 0x09	/* rotate left */
#define INS_ROR		INS_ARITH | 0x0A	/* rotate right */

/* INS_LOGIC group */
#define INS_AND		INS_LOGIC | 0x01
#define INS_OR		INS_LOGIC | 0x02
#define INS_XOR		INS_LOGIC | 0x03
#define INS_NOT		INS_LOGIC | 0x04
#define INS_NEG		INS_LOGIC | 0x05

/* INS_STACK group */
#define INS_PUSH		INS_STACK | 0x01
#define INS_POP		INS_STACK | 0x02
#define INS_PUSHREGS	INS_STACK | 0x03	/* push register context */
#define INS_POPREGS	INS_STACK | 0x04	/* pop register context */
#define INS_PUSHFLAGS	INS_STACK | 0x05	/* push all flags */
#define INS_POPFLAGS	INS_STACK | 0x06	/* pop all flags */
#define INS_ENTER		INS_STACK | 0x07	/* enter stack frame */
#define INS_LEAVE		INS_STACK | 0x08	/* leave stack frame */

/* INS_COND group */
#define INS_TEST		INS_COND | 0x01
#define INS_CMP		INS_COND | 0x02

/* INS_LOAD group */
#define INS_MOV		INS_LOAD | 0x01
#define INS_MOVCC		INS_LOAD | 0x02
#define INS_XCHG		INS_LOAD | 0x03
#define INS_XCHGCC	INS_LOAD | 0x04

/* INS_ARRAY group */
#define INS_STRCMP	INS_ARRAY | 0x01
#define INS_STRLOAD	INS_ARRAY | 0x02
#define INS_STRMOV	INS_ARRAY | 0x03
#define INS_STRSTOR	INS_ARRAY | 0x04
#define INS_XLAT		INS_ARRAY | 0x05

/* INS_BIT group */
#define INS_BITTEST	INS_BIT | 0x01
#define INS_BITSET	INS_BIT | 0x02
#define INS_BITCLR	INS_BIT | 0x03

/* INS_FLAG group */
#define INS_CLEARCF	INS_FLAG | 0x01	/* clear Carry flag */
#define INS_CLEARZF	INS_FLAG | 0x02	/* clear Zero flag */
#define INS_CLEAROF	INS_FLAG | 0x03	/* clear Overflow flag */
#define INS_CLEARDF	INS_FLAG | 0x04	/* clear Direction flag */
#define INS_CLEARSF	INS_FLAG | 0x05	/* clear Sign flag */
#define INS_CLEARPF	INS_FLAG | 0x06	/* clear Parity flag */
#define INS_SETCF		INS_FLAG | 0x07
#define INS_SETZF		INS_FLAG | 0x08
#define INS_SETOF		INS_FLAG | 0x09
#define INS_SETDF		INS_FLAG | 0x0A
#define INS_SETSF		INS_FLAG | 0x0B
#define INS_SETPF		INS_FLAG | 0x0C
#define INS_TOGCF		INS_FLAG | 0x10	/* toggle */
#define INS_TOGZF		INS_FLAG | 0x20
#define INS_TOGOF		INS_FLAG | 0x30
#define INS_TOGDF		INS_FLAG | 0x40
#define INS_TOGSF		INS_FLAG | 0x50
#define INS_TOGPF		INS_FLAG | 0x60

/* INS_FPU */

/* INS_TRAP */
#define INS_TRAP		INS_TRAPS | 0x01		/* generate trap */
#define INS_TRAPCC	INS_TRAPS | 0x02		/* conditional trap gen */
#define INS_TRET		INS_TRAPS | 0x03		/* return from trap */
#define INS_BOUNDS	INS_TRAPS | 0x04		/* gen bounds trap */
#define INS_DEBUG		INS_TRAPS | 0x05		/* gen breakpoint trap */
#define INS_TRACE		INS_TRAPS | 0x06		/* gen single step trap */
#define INS_INVALIDOP	INS_TRAPS | 0x07		/* gen invalid instruction */
#define INS_OFLOW		INS_TRAPS | 0x08		/* gen overflow trap */

/* INS_SYSTEM */
#define INS_HALT		INS_SYSTEM | 0x01		/* halt machine */
#define INS_IN		INS_SYSTEM | 0x02		/* input form port */
#define INS_OUT		INS_SYSTEM | 0x03		/* output to port */
#define INS_CPUID		INS_SYSTEM | 0x04		/* identify cpu */

/* INS_OTHER */
#define INS_NOP		INS_OTHER | 0x01
#define INS_BCDCONV	INS_OTHER | 0x02	/* convert to/from BCD */
#define INS_SZCONV	INS_OTHER | 0x03	/* convert size of operand */
 
   /* instruction size */
#define INS_BYTE      0x10000   /* operand is  8 bits/1 byte  */
#define INS_WORD      0x20000   /* operand is 16 bits/2 bytes */
#define INS_DWORD      0x40000   /* operand is 32 bits/4 bytes */
#define INS_QWORD      0x80000   /* operand is 64 bits/8 bytes */
   /* instruction modifiers */
#define INS_REPZ     0x0100000
#define INS_REPNZ    0x0200000  
#define INS_LOCK     0x0400000 /* lock bus */
#define INS_DELAY    0x0800000 /* branch delay slot */

#define INS_TYPE_MASK	0xFFFF
#define INS_GROUP_MASK	0x1000
#define INS_SIZE_MASK   0xF0000
#define INS_MOD_MASK    0xFF00000

   /* code patterns */
#define FUNCTION_PROLOGUE 0x0001
#define FUNCTION_EPILOGUE 0x0002

/* these could reuse OP types, but those types are too general... */
#define ADDEXP_SCALE_MASK  0x000000FF
#define ADDEXP_INDEX_MASK  0x0000FF00
#define ADDEXP_BASE_MASK   0x00FF0000
#define ADDEXP_DISP_MASK   0xFF000000
#define ADDEXP_SCALE_OFFSET 0
#define ADDEXP_INDEX_OFFSET 8
#define ADDEXP_BASE_OFFSET  16
#define ADDEXP_DISP_OFFSET  24
#define ADDREXP_BYTE    0x01
#define ADDREXP_WORD    0x02
#define ADDREXP_DWORD   0x03     
#define ADDREXP_QWORD   0x04
#define ADDREXP_REG     0x10 /*0x00 implies non-register */
#define AddrExp_ScaleType(x) x & ADDEXP_SCALE_MASK
#define AddrExp_IndexType(x) (x & ADDEXP_INDEX_MASK) >> 8
#define AddrExp_BaseType(x) (x & ADDEXP_BASE_MASK) >> 16
#define AddrExp_DispType(x) (x & ADDEXP_DISP_MASK) >> 24

/* pointless defines for fixing i386.c */
#define CODE_RVA 0

#endif
int vm_add_regtbl_entry( int index, char *name, int size, int type);
int addrexp_new(int scale,int index,int base,int disp,int flags);
int bdb_index_find(int, void *, void *);
int bdb_index_next(int, void *);


