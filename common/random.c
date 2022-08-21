#include "platform.h"
#include "random.h"

static uint64_t next;

void random_init(uint32_t seed)
{
  next = seed;
}

uint32_t random_get(void)
{
  next = next * 0x5851f42d4c957f2d + 1;
  return (unsigned int)(next >> 32);
}

