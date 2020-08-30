// PiGFX Project, Copyright 2020, Christian Lehner
// Memory management

#ifndef MEMORY_H__
#define MEMORY_H__

#define MEGABYTE 1024*1024
#define ARM_MEMSIZE 128*MEGABYTE            // won't need any more

#define KERNEL_MAX_SIZE		(2 * MEGABYTE)

#define KERNEL_STACK_SIZE	0x20000				// all sizes must be a multiple of 16K // 128KB
#define EXCEPTION_STACK_SIZE	0x8000          // 32KB
#define PAGE_TABLE1_SIZE	0x4000              // 16KB

#define MEM_KERNEL_START	0x8000
#define MEM_KERNEL_END		(MEM_KERNEL_START + KERNEL_MAX_SIZE)
#define MEM_KERNEL_STACK	(MEM_KERNEL_END + KERNEL_STACK_SIZE)		// expands down

#define MEM_ABORT_STACK		(MEM_KERNEL_STACK + EXCEPTION_STACK_SIZE)	// expands down
#define MEM_IRQ_STACK		(MEM_ABORT_STACK + EXCEPTION_STACK_SIZE)	// expands down
#define MEM_FIQ_STACK		(MEM_IRQ_STACK + EXCEPTION_STACK_SIZE)		// expands down

#define MEM_PAGE_TABLE1		MEM_FIQ_STACK				// must be 16K aligned
#define MEM_PAGE_TABLE1_END	(MEM_PAGE_TABLE1 + PAGE_TABLE1_SIZE)

#if RPI <= 3
// coherent memory region (one 1 MB section)
#define MEM_COHERENT_REGION	((MEM_PAGE_TABLE1_END + 2*MEGABYTE) & ~(MEGABYTE-1))

#define MEM_HEAP_START		(MEM_COHERENT_REGION + MEGABYTE)
#else
// coherent memory region (two 2 MB blocks)
#define MEM_COHERENT_REGION	((MEM_PAGE_TABLE1_END + 3*MEGABYTE) & ~(2*MEGABYTE-1))

#define MEM_HEAP_START		(MEM_COHERENT_REGION + 2*2*MEGABYTE)
#endif

#endif
