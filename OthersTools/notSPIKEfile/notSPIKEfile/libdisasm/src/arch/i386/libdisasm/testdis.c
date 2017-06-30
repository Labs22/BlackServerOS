/* Utility to test libdisasm. Disassembles from the start of a file.  */
/* Compile with  `gcc -I. -O3 -ggdb -L. -ldisasm testdis.c -o testdis` */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "libdis.h"

int main(int argc, char *argv[])
{
	int fTarget, i = 0, n, size;
	unsigned char *buf, out[20];
	void *image;
	off_t curr_pos;
	struct instr curr_inst;
	struct stat tmpStat;



	if (argc < 2) {
		printf("Usage: %s filename\n", argv[0]);
		return 1;
	}

	disassemble_init(0, ATT_SYNTAX);
	//disassemble_init(0, INTEL_SYNTAX);
	fTarget = open(argv[1], O_RDONLY);
	fstat(fTarget, &tmpStat);
	image = mmap(0, tmpStat.st_size, PROT_READ, MAP_SHARED, fTarget, 0);
	if ((int) image < 1)
		return (-1);
	buf = (unsigned char *) image;
	close(fTarget);
	printf("File name: %s\n", argv[1]);

	while (i < tmpStat.st_size) {
		memset(&curr_inst, 0, sizeof (struct code));
		/* test invariant */
		size = disasm_invariant( buf + i, tmpStat.st_size - i, out, 20 );
		printf("%X\t", i);
		for ( n = 0; n < size; n++ ) {
			printf("%02X ", out[n]);
		}
		printf("\t\t\t;invariant bytes (signature)\n");

		/* test code */
		printf("%X\t", i);
		size = disassemble_address(buf + i, &curr_inst);
		if (size) {
			for (n = 0; n < 12; n++) {
				if (n < size)
					printf("%02X ", buf[i + n]);
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
	munmap(image, tmpStat.st_size);
	disassemble_cleanup();
	return 0;
}
