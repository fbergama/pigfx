// Based on the uspi project
// 2020 Christian Lehner

#include "mmu.h"
#include "memory.h"
#include "synchronize.h"

void CreatePageTable(unsigned int nMemSize)
{
    // PageTable must be 16K aligned
	unsigned int* pPageTable = (unsigned int*)MEM_PAGE_TABLE1;

	for (unsigned int nEntry = 0; nEntry < 4096; nEntry++)
	{
		unsigned int nBaseAddress = MEGABYTE * nEntry;

#if RPI == 3
		if (nBaseAddress == MEM_COHERENT_REGION)
		{
			pPageTable[nEntry] = ARMV6MMUL1SECTION_COHERENT | nBaseAddress;
		}
		else
#endif
		if (nBaseAddress < nMemSize)
		{
			extern unsigned char _etext;
			if (nBaseAddress < (unsigned int) &_etext)
			{
				pPageTable[nEntry] = ARMV6MMUL1SECTION_NORMAL | nBaseAddress;
			}
			else
			{
				pPageTable[nEntry] = ARMV6MMUL1SECTION_NORMAL_XN | nBaseAddress;
			}
		}
		else
		{
			pPageTable[nEntry] = ARMV6MMUL1SECTION_DEVICE | nBaseAddress;
		}
	}

	CleanDataCache ();
	DataSyncBarrier ();
}

void EnableMMU()
{
	unsigned int nAuxControl;
	asm volatile ("mrc p15, 0, %0, c1, c0,  1" : "=r" (nAuxControl));
#if RPI == 1
	nAuxControl |= ARM_AUX_CONTROL_CACHE_SIZE;	// restrict cache size (no page coloring)
#else
	nAuxControl |= ARM_AUX_CONTROL_SMP;
#endif
	asm volatile ("mcr p15, 0, %0, c1, c0,  1" : : "r" (nAuxControl));

	// set TTB control
	asm volatile ("mcr p15, 0, %0, c2, c0,  2" : : "r" (TTBCR_SPLIT));

	// set TTBR0
	asm volatile ("mcr p15, 0, %0, c2, c0,  0" : : "r" (MEM_PAGE_TABLE1));

	// set Domain Access Control register (Domain 0 and 1 to client)
	asm volatile ("mcr p15, 0, %0, c3, c0,  0" : : "r" (DOMAIN_CLIENT << 0));

	InvalidateDataCache ();
	FlushPrefetchBuffer ();

	// enable MMU
	unsigned int nControl;
	asm volatile ("mrc p15, 0, %0, c1, c0,  0" : "=r" (nControl));
#if RPI == 1
#ifdef ARM_STRICT_ALIGNMENT
	nControl &= ~ARM_CONTROL_UNALIGNED_PERMITTED;
	nControl |= ARM_CONTROL_STRICT_ALIGNMENT;
#else
	nControl &= ~ARM_CONTROL_STRICT_ALIGNMENT;
	nControl |= ARM_CONTROL_UNALIGNED_PERMITTED;
#endif
#endif
	nControl |= MMU_MODE;
	asm volatile ("mcr p15, 0, %0, c1, c0,  0" : : "r" (nControl) : "memory");
}
