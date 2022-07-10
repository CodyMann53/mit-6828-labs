/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_TRAP_H
#define JOS_KERN_TRAP_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/trap.h>
#include <inc/mmu.h>

void _divide_error();
void _debug();
void _nmi();
void _breakpoint();
void _overflow();
void _bound();
void _illegal_opcode();
void _dev_not_avail();
void _double_fault();
void _invl_tss();
void _seg_not_present();
void _stack();
void _gen_prot();
void _page_fault();
void _floating_point();
void _alignment_check();
void _machine_check();
void _simd_floating_point();
void _syscall();

/* The kernel's interrupt descriptor table */
extern struct Gatedesc idt[];
extern struct Pseudodesc idt_pd;

void trap_init(void);
void trap_init_percpu(void);
void print_regs(struct PushRegs *regs);
void print_trapframe(struct Trapframe *tf);
void page_fault_handler(struct Trapframe *);
void backtrace(struct Trapframe *);

#endif /* JOS_KERN_TRAP_H */
