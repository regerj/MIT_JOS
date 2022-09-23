// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

// LAB 1: add your command to here...
static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "show", "Show a cute animation :)", mon_show },
	{ "backtrace", "Display a stack backtrace", mon_backtrace },
};

/***** Implementations of basic kernel monitor commands *****/

int mon_show(int argc, char **argv, struct Trapframe *tf)
{
	// cprintf("\033[0;30m"); 	// Black
	// cprintf("\033[0;31m"); 	// Red
	// cprintf("\033[0;32m"); 	// Green
	// cprintf("\033[0;33m"); 	// Yellow
	// cprintf("\033[0;34m"); 	// Blue
	// cprintf("\033[0;35m"); 	// Purple
	// cprintf("\033[0;36m"); 	// Cyan
	// cprintf("\033[0;37m"); 	// White

	int printOS2()
	{
		cprintf(" ________  ________    _______     \n");
		cprintf("|\\   __  \\|\\   ____\\  /  ___  \\    \n");
		cprintf("\\ \\  \\|\\  \\ \\  \\___|_/__/|_/  /|   \n");
		cprintf(" \\ \\  \\\\\\  \\ \\_____  \\__|//  / /   \n");
		cprintf("  \\ \\  \\\\\\  \\|____|\\  \\  /  /_/__  \n");
		cprintf("   \\ \\_______\\____\\_\\  \\|\\________\\\n");
		cprintf("    \\|_______|\\_________\\\\|_______|\n");
		cprintf("             \\|_________|          \n");
		return 0;
	}

	int printCat()
	{
		cprintf("            *     ,MMM8&&&.            *\n");
		cprintf("                  MMMM88&&&&&    .\n");
		cprintf("                 MMMM88&&&&&&&\n");
		cprintf("     *           MMM88&&&&&&&&\n");
		cprintf("                 MMM88&&&&&&&&\n");
		cprintf("                 'MMM88&&&&&&'\n");
		cprintf("                   'MMM8&&&'      *    \n");
		cprintf("          |\\___/|     /\\___/\\\n");
		cprintf("          )     (     )    ~( .              '\n");
		cprintf("         =\\     /=   =\\~    /=\n");
		cprintf("           )===(       ) ~ (\n");
		cprintf("          /     \\     /     \\\n");
		cprintf("          |     |     ) ~   (\n");
		cprintf("         /       \\   /     ~ \\\n");
		cprintf("         \\       /   \\~     ~/\n");
		cprintf("  /\\_/\\_/\\__  _/_/\\_/\\__~__/_/\\_/\\_/\\_/\\_/\\_\n");
		cprintf("  |  |  |  |( (  |  |  | ))  |  |  |  |  |  |\n");
		cprintf("  |  |  |  | ) ) |  |  |//|  |  |  |  |  |  |\n");
		cprintf("  |  |  |  |(_(  |  |  (( |  |  |  |  |  |  |\n");
		cprintf("  |  |  |  |  |  |  |  |\\)|  |  |  |  |  |  |\n");
		cprintf("  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |\n");
		return 0;
	}

	cprintf("\033[0;31m"); 	// Red
	printOS2();
	printCat();
	cprintf("\033[0;32m"); 	// Green
	printOS2();
	printCat();
	cprintf("\033[0;33m"); 	// Yellow
	printOS2();
	printCat();
	cprintf("\033[0;34m"); 	// Blue
	printOS2();
	printCat();
	cprintf("\033[0;35m"); 	// Purple
	printOS2();
	printCat();
	cprintf("\033[0;36m"); 	// Cyan
	printOS2();
	printCat();
	cprintf("\033[0;37m"); 	// White
	printOS2();
	printCat();
	return 0;
}

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// LAB 1: Your code here.
    // HINT 1: use read_ebp().
    // HINT 2: print the current ebp on the first line (not current_ebp[0])

	// Declare and read in the ebp
	uint32_t * ebp;
	ebp = (uint32_t *)read_ebp();

	// Set a loop to iterate the ebps until you hit base.
	while(ebp != 0x00000000)
	{
		cprintf("ebp %08x eip %08x args %08x %08x %08x %08x %08x\n", (int)ebp, (int)*(ebp + 1), 
				(int)*(ebp + 2), (int)*(ebp + 3), (int)*(ebp + 4), (int)*(ebp + 5), 
				(int)*(ebp + 6));

		// Create and retrieve an info struct for the current eip (*(ebp + 1))
		struct Eipdebuginfo info;
		debuginfo_eip((uintptr_t)(*(ebp + 1)), &info);

		// Print it all pretty like
		cprintf("Line number: %d", info.eip_line);
		cprintf("\t%s:%d: %.*s+%d\n", info.eip_file, info.eip_line, info.eip_fn_namelen, info.eip_fn_name, (ebp - ebp[1]));
		ebp = (uint32_t *)*ebp;
	}

	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
