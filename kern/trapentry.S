/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(_divide_error, T_DIVIDE)
TRAPHANDLER_NOEC(_debug, T_DEBUG)
TRAPHANDLER_NOEC(_nmi, T_NMI)
TRAPHANDLER_NOEC(_breakpoint, T_BRKPT)
TRAPHANDLER_NOEC(_overflow, T_OFLOW)
TRAPHANDLER_NOEC(_bound, T_BOUND)
TRAPHANDLER_NOEC(_illegal_opcode, T_ILLOP)
TRAPHANDLER_NOEC(_dev_not_avail, T_DEVICE)
TRAPHANDLER(_double_fault, T_DBLFLT)
TRAPHANDLER(_invl_tss, T_TSS)
TRAPHANDLER(_seg_not_present, T_SEGNP)
TRAPHANDLER(_stack, T_STACK)
TRAPHANDLER(_gen_prot, T_GPFLT)
TRAPHANDLER(_page_fault, T_PGFLT)
TRAPHANDLER_NOEC(_floating_point, T_FPERR)
TRAPHANDLER(_alignment_check, T_ALIGN)
TRAPHANDLER_NOEC(_machine_check, T_MCHK)
TRAPHANDLER_NOEC(_simd_floating_point, T_SIMDERR)
TRAPHANDLER_NOEC(_syscall, T_SYSCALL)

/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	pushl %ds
	pushl %es
	pushal
        movw $GD_KD, %ax
	movw %ax, %es
	movw %ax, %ds
        pushl %esp
        call trap

_fail:
	jmp _fail
