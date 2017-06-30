/* A quick, dirty, stupid disassembler to test the engine */
/* Compile with  `gcc -I. -O3 -ggdb -L. -ldisasm quikdis.c -o quikdis` */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <elf.h>
#include "libdis.h"

int main(int argc, char *argv[])
{
	int fTarget, entry, x, i = 0, n, size;
	unsigned char *buf;
	void *image;
	off_t curr_pos;
	Elf32_Ehdr *TargetHeader;
	Elf32_Phdr *ProgramHeader;
	struct instr curr_inst;
	struct stat tmpStat;



	if (argc < 2) {
		printf("Usage: %s filename\n", argv[0]);
		return 1;
	}

	disassemble_init(0, INTEL_SYNTAX);
	/* read ELF header */
	fTarget = open(argv[1], O_RDONLY);
	fstat(fTarget, &tmpStat);
	image = mmap(0, tmpStat.st_size, PROT_READ, MAP_SHARED, fTarget, 0);
	if ((int) image < 1)
		return (-1);
	close(fTarget);
	printf("File name: %s\n", argv[1]);
	TargetHeader = image;

	/* read program header table */
	for (x = 0; x < TargetHeader->e_phnum; x++) {
		ProgramHeader =
		    image + TargetHeader->e_phoff +
		    (x * TargetHeader->e_phentsize);
		/* IF entry point is in this section */
		if (TargetHeader->e_entry >= ProgramHeader->p_vaddr &&
		    TargetHeader->e_entry <=
		    (ProgramHeader->p_vaddr + ProgramHeader->p_filesz)) {
			/* resolve entry point RVA to a file offset */
			entry = TargetHeader->e_entry -
			    (ProgramHeader->p_vaddr - ProgramHeader->p_offset);
			printf
			    ("\tDisassembling from entry point at offset %X\n",
			     entry);

			/* read entire program segment into buffer */
			buf = image + entry;
			while (i < (ProgramHeader->p_filesz - entry)) {
				memset(&curr_inst, 0, sizeof (struct code));
				printf("%X\t", TargetHeader->e_entry + i);
				size = disassemble_address(buf + i, &curr_inst);
				if (size) {
					for (n = 0; n < 12; n++) {
						if (n < size)
							printf("%02X ",
							       buf[i + n]);
						else
							printf("   ");
					}
					printf("%s", curr_inst.mnemonic);
					if (curr_inst.dest[0] != 0)
						printf("\t%s", curr_inst.dest);
					if (curr_inst.src[0] != 0)
						printf(", %s", curr_inst.src);
					if (curr_inst.aux[0] != 0)
						printf(", %s", curr_inst.aux);
					printf("\n");
					i += size;
				} else {
					printf("invalid opcode %02X\n", buf[i]);
					i++;
				}
			}
		}
	}
	munmap(image, tmpStat.st_size);
	disassemble_cleanup();
	return 0;
}
