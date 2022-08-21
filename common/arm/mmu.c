#include "platform.h"


#include "mmu.h"

#define DCACHEROWSIZE 64

extern unsigned char __mmu_start[];
extern unsigned char __mmu_end[];
extern unsigned char __dma_start[];
extern unsigned char __dma_end[];

enum {
	MAP_TYPE_NCNB = 0x0, // strongly ordered
	MAP_TYPE_NCB  = 0x1, // shareable device
	MAP_TYPE_CNB  = 0x2, // write through cache
	MAP_TYPE_CB   = 0x3, // write back cache
};

static void map_l1_section(uint32_t * ttb, uintptr_t virt, uintptr_t phys, size_t size, int type)
{
	size_t i;

	virt >>= 20;
	phys >>= 20;
	size >>= 20;
	type &= 0x3;

	for(i = size; i > 0; i--, virt++, phys++) {
		ttb[virt] = (phys << 20) | (1 << 16) | (0x3 << 10) | (0x0 << 5) | (type << 2) | (0x2 << 0);
	}
}

#define SZ_128M                         (0x08000000)
#define SZ_2G                           (0x80000000)

void mmu_setup(void)
{
	uint32_t * ttb = (uint32_t *)__mmu_start;

	map_l1_section(ttb, 0x00000000, 0x00000000, SZ_2G, 0);
	map_l1_section(ttb, 0x80000000, 0x80000000, SZ_2G, 0);
	map_l1_section(ttb, 0x40000000, 0x40000000, SZ_128M, MAP_TYPE_CB);
	map_l1_section(ttb, (uintptr_t)__dma_start, (uintptr_t)__dma_start, (size_t)(__dma_end - __dma_start), MAP_TYPE_NCNB);
}

void mmu_enable(void)
{
	uintptr_t table = (uintptr_t)__mmu_start;

	MMU_Disable();
	L1C_DisableCaches();
	L1C_DisableBTAC();

	// Invalidate entire Unified TLB
	MMU_InvalidateTLB();

	// Invalidate entire branch predictor array
	L1C_InvalidateBTAC();

	//  Invalidate instruction cache and flush branch target cache
	L1C_InvalidateICacheAll();
	L1C_InvalidateDCacheAll();

	__set_TTBR0(table | 0x48);

	//  Invalidate data cache
	L1C_InvalidateDCacheAll();

	MMU_InvalidateTLB();

	__set_DACR(0xffffffff);
	__ISB();

	MMU_Enable();

	L1C_EnableCaches();
	L1C_EnableBTAC();

	__set_ACTLR(__get_ACTLR() | ACTLR_SMP_Msk);
//	L2C_Enable();
}

void L1_CleanDCache_by_Addr(volatile void *addr, int32_t dsize)
{
	if (dsize > 0) {
		int32_t op_size = dsize + (((uintptr_t) addr) & (DCACHEROWSIZE - 1U));
		uintptr_t op_mva = (uintptr_t) addr & ~(DCACHEROWSIZE - 1U);
		__DSB();

		do {
			__set_DCCMVAC(op_mva);  // Clean data cache line by address.
			op_mva += DCACHEROWSIZE;
			op_size -= DCACHEROWSIZE;
		} while (op_size > 0);
		__DMB();     // ensure the ordering of data cache maintenance operations and their effects
	}
}

void L1_CleanInvalidateDCache_by_Addr(volatile void *addr, int32_t dsize)
{
	if (dsize > 0) {
		int32_t op_size = dsize + (((uintptr_t) addr) & (DCACHEROWSIZE - 1U));
		uintptr_t op_mva = (uintptr_t) addr & ~(DCACHEROWSIZE - 1U);
		__DSB();

		do {
			__set_DCCIMVAC(op_mva); // Clean and Invalidate data cache by address.
			op_mva += DCACHEROWSIZE;
			op_size -= DCACHEROWSIZE;
		} while (op_size > 0);
		__DMB();     // ensure the ordering of data cache maintenance operations and their effects
	}
}

void L1_InvalidateDCache_by_Addr(volatile void *addr, int32_t dsize)
{
	if (dsize > 0) {
		int32_t op_size = dsize + (((uintptr_t) addr) & (DCACHEROWSIZE - 1U));
		uintptr_t op_mva = (uintptr_t) addr & ~(DCACHEROWSIZE - 1U);

		do {
			__set_DCIMVAC(op_mva);  // Invalidate data cache line by address.
			op_mva += DCACHEROWSIZE;
			op_size -= DCACHEROWSIZE;
		} while (op_size > 0);

		// Cache Invalidate operation is not follow by memory-writes
		__DMB();     // ensure the ordering of data cache maintenance operations and their effects
	}
}


