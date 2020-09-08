//
// mmu.c
// Pagetable and MMU
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2020 Christian Lehner
// Based on the USPI project at
// https://github.com/rsta2/uspi

#ifndef MMU_H__
#define MMU_H__

// Domains
#define DOMAIN_NO_ACCESS	0
#define DOMAIN_CLIENT		1
#define DOMAIN_MANAGER		3

// First 2 bits define the type of section
#define FIRST_LVL_BUFFERABLE (1 << 2)
#define FIRST_LVL_CACHEABLE  (1 << 3)
#define FIRST_LVL_EXECUTE_NEVER  (1 << 4)
// bits 5 to 8 define the domain
#define FIRST_LVL_SHAREABLE  (1 << 16)

#define ARMV6MMUL1SECTION_NORMAL	0x1040E		//	+ shareable                         1 0000 0100 0000 1110
#define ARMV6MMUL1SECTION_NORMAL_XN	0x1041E		//	+ shareable and execute never       1 0000 0100 0001 1110
#define ARMV6MMUL1SECTION_NORMAL_NS 0x1040A					//	normal cache no share   1 0000 0100 0000 1010
#define ARMV6MMUL1SECTION_DEVICE	0x10416		// shared device                        1 0000 0100 0001 0110
#define ARMV6MMUL1SECTION_COHERENT	0x10412		// strongly ordered                     1 0000 0100 0001 0010

// (System) Control register
#define ARM_CONTROL_MMU			(1 << 0)
#define ARM_CONTROL_STRICT_ALIGNMENT	(1 << 1)
#define ARM_CONTROL_L1_CACHE		(1 << 2)
#define ARM_CONTROL_BRANCH_PREDICTION	(1 << 11)
#define ARM_CONTROL_L1_INSTRUCTION_CACHE (1 << 12)
#if RPI == 1
#define ARM_CONTROL_UNALIGNED_PERMITTED	(1 << 22)
#define ARM_CONTROL_EXTENDED_PAGE_TABLE	(1 << 23)
#endif

//  Auxiliary Control register
#if RPI == 1
#define ARM_AUX_CONTROL_CACHE_SIZE	(1 << 6)	// restrict cache size to 16K (no page coloring)
#else
#define ARM_AUX_CONTROL_SMP		(1 << 6)
#endif

#if RPI == 1
#define MMU_MODE	(  ARM_CONTROL_MMU			\
			 | ARM_CONTROL_L1_CACHE			\
			 | ARM_CONTROL_L1_INSTRUCTION_CACHE	\
			 | ARM_CONTROL_BRANCH_PREDICTION	\
			 | ARM_CONTROL_EXTENDED_PAGE_TABLE)
#else
#define MMU_MODE	(  ARM_CONTROL_MMU			\
			 | ARM_CONTROL_L1_CACHE			\
			 | ARM_CONTROL_L1_INSTRUCTION_CACHE	\
			 | ARM_CONTROL_BRANCH_PREDICTION)
#endif

#define TTBCR_SPLIT	0
#define ARMV6MMU_FAULT 0x0000

void CreatePageTable(unsigned int nMemSize);
void EnableMMU();

#endif // MMU_H__
