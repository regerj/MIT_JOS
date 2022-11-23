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

void myHook()
{
	cprintf("In hook xd\n\n\n\n\n\n\n");
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

	//struct idtr idt;
	char idta[6];
    asm volatile ("sidt %0" : "=m"(idta));
	uintptr_t * base = (uintptr_t *)&idta[2];
	unsigned short * limit = (unsigned short *)idta;

	struct idtr idtr;
	idtr.base = *base;
	idtr.limit = *limit;
    cprintf("Found an IDT with address: %08x and size: %04x\n", idtr.base, idtr.limit);

	struct Gatedesc * idt = (struct Gatedesc *)idtr.base;

	uintptr_t func_addr = gatedesc2addr(&idt[T_SYSCALL]);
	func_addr = 0xf0103a00;
	addr2gatedesc(&idt[T_BRKPT], func_addr);

	asm volatile("int $3");
	cprintf("Pulled hook\n");

	// cprintf("My hook has address: %08x\n", myHook);
	// addr2gatedesc(&idt[T_SYSCALL], func_addr);
	// cprintf("Hook complete\n");
	// cprintf("The syscall entry of the IDT is: %016x\n", idt[T_SYSCALL]);

	// save the name of the program so that panic() can use it
	if (argc > 0)
		binaryname = argv[0];

	// call user main routine
	umain(argc, argv);

	// exit gracefully
	exit();
}

