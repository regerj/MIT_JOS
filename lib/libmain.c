// Called from entry.S to get us going.
// entry.S already took care of defining envs, pages, uvpd, and uvpt.

#include <inc/lib.h>

extern void umain(int argc, char **argv);

const volatile struct Env *thisenv;
const char *binaryname = "<unknown>";

struct idtr {
    uintptr_t base;
    unsigned short limit;
};

uintptr_t gatedesc2addr(struct Gatedesc * idte)
{
	// cprintf("gd_off_15_0: %04x\n", idte->gd_off_15_0);
	// cprintf("gd_off_31_16: %08x\n", idte->gd_off_31_16);
	uintptr_t addr = idte->gd_off_15_0 | (idte->gd_off_31_16 << 16);
	// cprintf("Reconstructed address: %08x\n", addr);
	return addr;
}

void addr2gatedesc(struct Gatedesc * idte, uintptr_t addr)
{
	uintptr_t gd_off_15_0 = addr & 0xffff;
	uintptr_t gd_off_31_16 = (addr >> 16);
	// cprintf("gd_off_15_0: %04x\n", gd_off_15_0);
	// cprintf("gd_off_31_16: %08x\n", gd_off_31_16);
	idte->gd_off_15_0 = gd_off_15_0;
	idte->gd_off_31_16 = gd_off_31_16;
}

// // Gate descriptors for interrupts and traps
// struct Gatedesc {
// 	unsigned gd_off_15_0 : 16;   // low 16 bits of offset in segment
// 	unsigned gd_sel : 16;        // segment selector
// 	unsigned gd_args : 5;        // # args, 0 for interrupt/trap gates
// 	unsigned gd_rsv1 : 3;        // reserved(should be zero I guess)
// 	unsigned gd_type : 4;        // type(STS_{TG,IG32,TG32})
// 	unsigned gd_s : 1;           // must be 0 (system)
// 	unsigned gd_dpl : 2;         // descriptor(meaning new) privilege level
// 	unsigned gd_p : 1;           // Present
// 	unsigned gd_off_31_16 : 16;  // high bits of offset in segment
// };

void
libmain(int argc, char **argv)
{
	// set thisenv to point at our Env structure in envs[].
	// LAB 3: Your code here.
	thisenv = &envs[ENVX(sys_getenvid())];

	// Read and format idtr from the CPU
	struct idtr idtr;
	char idt_buff[6];								// 6 bytes for holding the idtr
    asm volatile ("sidt %0" : "=m"(idt_buff));		// Grab the idtr itself
	idtr.base = *(uintptr_t *)&idt_buff[2];			// Isolate and set base
	idtr.limit = *(unsigned short *)&idt_buff[0];	// Isolate and set limit

    cprintf("Found an IDT with address: %08x and size: %04x\n", idtr.base, idtr.limit);

	// Grab specifically the IDT itself
	struct Gatedesc * idt = (struct Gatedesc *)idtr.base;

	// Hardcoded address of our hook manually mapped into kernel
	uintptr_t func_addr = 0xf0103a00;

	// This func overwrites the IDT with our hardcoded func
	addr2gatedesc(&idt[T_BRKPT], func_addr);

	// Set some argument to our hook passed via %ebx
	uint32_t myebx = 0xdeadbeef;
	// uint32_t myebx2 = 0x00000000;
	asm volatile("movl %0,%%ebx" : : "r" (myebx));
	// asm volatile("movl %%ebx,%0" : "=r" (myebx2));

	// cprintf("myebx2: %08x\n", myebx2);

	// Pull the hook
	asm volatile("push $0x8000e9");
	asm volatile("int $3");
	cprintf("Pulled hook\n");

	// save the name of the program so that panic() can use it
	if (argc > 0)
		binaryname = argv[0];

	// call user main routine
	umain(argc, argv);

	// exit gracefully
	exit();
}

