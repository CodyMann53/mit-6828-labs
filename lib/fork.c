// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;
	int pn = ((uint32_t)(addr)) / PGSIZE;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if (!(err & FEC_WR)) 
	{
		panic("Invalid call to user page fault handler. No write.");	
	}

	if (!(uvpt[pn] & PTE_COW))
	{
		panic("Invalid call to user page fault handler. Page must be copy on write.");
	}
		

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	int retVal = sys_page_alloc(0, PFTEMP, PTE_U | PTE_P | PTE_W);
	if (retVal != 0)
	{
		panic("sys_page_alloc: %e", retVal);
	}

	memcpy((void *) PFTEMP, addr, PGSIZE);
				
	retVal = sys_page_map(0, PFTEMP, 0, addr, PTE_U | PTE_P | PTE_W);
	if (retVal != 0)
	{
		panic("sys_page_map: %e", retVal);
	}


	retVal = sys_page_unmap(0, PFTEMP);
	if (retVal != 0)
	{
		panic("sys_page_map: %e", retVal);
	}
	return;
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	if ((uvpt[pn] & PTE_W) || (uvpt[pn] & PTE_COW))
	{
		r = sys_page_map(0, (void *)(pn*PGSIZE), envid, (void *)(pn*PGSIZE), PTE_U | PTE_P | PTE_COW);
		if (r < 0)
		{
			return r;
		}
		
		r = sys_page_map(0, (void *)(pn*PGSIZE), 0, (void *)(pn*PGSIZE), PTE_U | PTE_P | PTE_COW);
		if (r < 0)
		{
			return r;
		}
	
	}
	else if (uvpt[pn] & PTE_P)
	{
		r = sys_page_map(0, (void *)(pn*PGSIZE), envid, (void *)(pn*PGSIZE), PTE_U | PTE_P);
		if (r < 0)
		{
			return r;
		}
	}
	else
	{
		panic("Duplicating page that isn't present.");
	}
	
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	set_pgfault_handler(pgfault);
	
	envid_t id = sys_exofork();
	if (id < 0)
	{
		return id;
	}
	
	if (id == 0)
	{
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	
	for (int i = 0; i < USTACKTOP / PGSIZE; i++)
	{
		if ((uvpt[i] & PTE_P))
		{
			int retVal = duppage(id, i);
			if (retVal < 0)
			{
				panic("dupage failure: %e", retVal);
			}
		}	
	}	
	
	int retVal = sys_page_alloc(id, (void *) (UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W);
	if (retVal != 0)
	{
		panic("sys_page_alloc: %e\n", retVal);
	}

	retVal = sys_env_set_pgfault_upcall(id, thisenv->env_pgfault_upcall);
	if (retVal != 0)
	{
		panic("sys_env_set_pgfault_upcall: %e\n", retVal);
	}
	
	retVal = sys_env_set_status(id, ENV_RUNNABLE);
	if (retVal != 0)
	{
		return retVal;
	}
 
	return id;			
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
