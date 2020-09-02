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

#define ARMV6MMUL1SECTION_NORMAL	0x0040E		// outer and inner write back, no write allocate
#define ARMV6MMUL1SECTION_NORMAL_XN	0x0041E		// 	+ execute never
#define ARMV6MMUL1SECTION_DEVICE	0x10416		// shared device
#define ARMV6MMUL1SECTION_COHERENT	0x10412		// strongly ordered

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

#if RASPPI == 1
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
