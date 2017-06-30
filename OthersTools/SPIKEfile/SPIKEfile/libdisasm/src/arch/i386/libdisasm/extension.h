#ifndef EXTENSION_H
#define EXTENSION_H
typedef void (*ext_init_fn)(void *);
typedef void (*ext_clean_fn)(void);
struct EXTENSION {
   char *filename;      /* name of extension file [full path] */
   int flags;           /* uhh..... */
   void *lib;           /* pointer to library */
   ext_init_fn fn_init;       /* init function for extension */
   ext_clean_fn fn_cleanup;    /* cleanup function for extension */
}; /* these are linked to from standard extensions : hll, engine, etc */

/* predef 's for use with dynamic loading */
typedef int (*disfunc_fn)(unsigned char *, unsigned char, struct code *, long);
typedef int (*getcode_fn)(struct code **);
struct EXT__ARCH {   /* disassembler information */
   struct EXTENSION ext;
   //char name[PATH_MAX];      // name of disassembler lib [lib(target.cpu).so]
   int options;             // module-specific options
   /* ------------------  CPU Information  -------------------- */
   int cpu_hi, cpu_lo;      // CPU high and low version numbers
   char endian;             // 0 = BIG, 1 = LITTLE
   char sz_addr;            // Default Size of Address in Bytes
   char sz_oper;            // Default Size of Operand in Bytes
   char sz_inst;            // Default Size of Instruction in Bytes
   char sz_byte;            // Size of Machine Byte in Bits
   char sz_word;            // Size of Machine Word in Bytes
   char sz_dword;           // Size of Machine DoubleWord in Bytes
   int SP;                  // RegID of Stack Pointer
   int IP;                  // RegID of Instruction Pointer
   int FP;                  // RegID of Frame Pointer
   int reg_gen;             // start of General regs in table
   int reg_seg, reg_fp;     // start of seg, FPU regs in table
   int reg_in, reg_out;     // start of procedure IN, OUT regs in table

   /* ------------------ Register Tables ---------------------- */
   struct REGTBL_ENTRY *reg_table;
   int sz_regtable;
   unsigned char *reg_storage;
   /* ------------------ Library Functions -------------------- */
   disfunc_fn fn_disasm_addr;      // ptr to disassembly routine
   getcode_fn fn_get_prologue;   // WARNING From here down are considered   
   getcode_fn fn_get_epilogue;   //         "optional"  
};
#endif
