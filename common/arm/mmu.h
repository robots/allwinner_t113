#ifndef MMU_h_
#define MMU_h_

void mmu_setup(void);
void mmu_enable(void);

void L1_CleanDCache_by_Addr(volatile void *addr, int32_t dsize);
void L1_CleanInvalidateDCache_by_Addr(volatile void *addr, int32_t dsize);
void L1_InvalidateDCache_by_Addr(volatile void *addr, int32_t dsize);

#endif
