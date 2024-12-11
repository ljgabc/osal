/**
 * @file osal_memory.h
 * @author ljgabc
 * @brief 动态内存管理器
 * 内存管理器申请了一个大数组theheap做为内存空间，然后把内存空间分成两个部分，
 * 第一个部分是针对小块内存的管理，第二部分是针对大块内存的管理。
 * 这样做的好处是容易申请到连续的大空间，因为小块内存处理会使整个内存空间碎片化，
 * 从而会导致内存空间不连续，不连续的空间是对申请大空间是非常不利的。
 * 在系统初始化阶段申请的内存(一般不会被释放的内存，成为常驻内存)也是在小块内存区域进行申请
 * @version 0.1
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "osal.h"

#define OSALMEM_IN_USE (0x1 << 31)

// 在32位MCU上，内存块头占用4个字节
#define OSALMEM_HDRSZ sizeof(osal_mem_hdr_t)

/**
 * @brief
 * 将一个数按OSALMEM_HDRSZ向上取整，比如输入1、2、3、4输出是4，输入5、6、7、8输出是8
 * 方便做内存字对齐
 */
#define OSALMEM_ROUND(X)                                                       \
  ((((X) + OSALMEM_HDRSZ - 1) / OSALMEM_HDRSZ) * OSALMEM_HDRSZ)

/**
 * @brief 当一个块剩余的内存小于此值时，再分割剩余的内存也很小了，可能用不上了
 * 分割出来太多小的内存块之后，内存申请的效率会降低
 * 但如果剩余的比较多，又会造成内存浪费，需要根据实际情况分配一个比较合理的值
 * 当块剩余内存小于该值时，不进行分割了，直接返回
 */
#ifndef OSALMEM_MIN_BLKSZ
#define OSALMEM_MIN_BLKSZ (OSALMEM_ROUND((OSALMEM_HDRSZ * 2)))
#endif

/**
 * @brief 小块内存区域的内存管理单元大小，当申请内存小于此值时在小块区域进行申请
 */
#if !defined OSALMEM_SMALL_BLKSZ
#define OSALMEM_SMALL_BLKSZ (OSALMEM_ROUND(16))
#endif

/**
 * @brief 小块内存区域的数量
 *
 */
#if !defined OSALMEM_SMALL_BLKCNT
#define OSALMEM_SMALL_BLKCNT 8
#endif

/**
 * @brief
 * 常驻内存区域大小，比如创建的线程、初始化阶段申请的内存，这些一般不会被释放，可以认为是常驻的内存
 *
 */
#if !defined OSALMEM_LL_BLKSZ
#define OSALMEM_LL_BLKSZ (OSALMEM_ROUND(6) + (1 * OSALMEM_HDRSZ))
#endif

/**
 * @brief 小块内存管理区域的总大小，包含常驻内存部分
 *
 */
#define OSALMEM_SMALLBLK_BUCKET                                                \
  ((OSALMEM_SMALL_BLKSZ * OSALMEM_SMALL_BLKCNT) + OSALMEM_LL_BLKSZ)

/**
 * @brief 两块内存管理区中间空出来一个块，设置为占用，避免两块区域之间混淆
 * OSALMEM_SMALLBLK_HDRCNT就代表了这个块的索引
 *
 */
#define OSALMEM_SMALLBLK_HDRCNT (OSALMEM_SMALLBLK_BUCKET / OSALMEM_HDRSZ)

/**
 * @brief 大块内存管理区域第一个块的索引
 *
 */
#define OSALMEM_BIGBLK_IDX (OSALMEM_SMALLBLK_HDRCNT + 1)

/**
 * @brief 大块内存管理区域的总大小，
 * 总内存大小减去小块内存区域，再减去两个头块（一个在两个内存中间，一个在整个内存尾部）
 */
#define OSALMEM_BIGBLK_SZ                                                      \
  (MAXMEMHEAP - OSALMEM_SMALLBLK_BUCKET - OSALMEM_HDRSZ * 2)

/**
 * @brief 内存最后一个块的索引，将其val设置为0，代表内存管理区域到头了
 *
 */
#define OSALMEM_LASTBLK_IDX ((MAXMEMHEAP / OSALMEM_HDRSZ) - 1)

#if OSALMEM_PROFILER
#define OSALMEM_INIT 'X'
#define OSALMEM_ALOC 'A'
#define OSALMEM_REIN 'F'
#endif

typedef union {
  uint32_t val;
  struct {
    // 低31位表示内存块的大小(包括头部分)
    unsigned len : 31;
    // 最高位表示内存块是否被使用
    unsigned inUse : 1;
  };
} osal_mem_hdr_t;

static osal_mem_hdr_t theHeap[MAXMEMHEAP / OSALMEM_HDRSZ];
static osal_mem_hdr_t *ff1; // First free block in the small-block bucket.
static uint8_t mem_stat;    // Discrete status flags: 0x01 = kicked.

#if OSALMEM_METRICS
static uint16_t blkMax;  // Max cnt of all blocks ever seen at once.
static uint16_t blkCnt;  // Current cnt of all blocks.
static uint16_t blkFree; // Current cnt of free blocks.
static uint16_t memAlo;  // Current total memory allocated.
static uint16_t memMax;  // Max total memory ever allocated at once.
#endif

#if OSALMEM_PROFILER
#define OSALMEM_PROMAX 8
/* The profiling buckets must differ by at least OSALMEM_MIN_BLKSZ; the
 * last bucket must equal the max alloc size. Set the bucket sizes to
 * whatever sizes necessary to show how your application is using memory.
 */
static uint16_t proCnt[OSALMEM_PROMAX] = {
    OSALMEM_SMALL_BLKSZ, 48, 112, 176, 192, 224, 256, 65535};
static uint16_t proCur[OSALMEM_PROMAX] = {0};
static uint16_t proMax[OSALMEM_PROMAX] = {0};
static uint16_t proTot[OSALMEM_PROMAX] = {0};
static uint16_t proSmallBlkMiss;
#endif

#ifdef DPRINTF_HEAPTRACE
extern int dprintf(const char *fmt, ...);
#endif /* DPRINTF_HEAPTRACE */

/*
 * 初始化内存管理器
 * 小块内存管理区域的len设置为OSALMEM_SMALLBLK_BUCKET
 * 大块内存管理区域的len设置为OSALMEM_BIGBLK_SZ
 * 中间一块内存设置为占用，用以分割两块区域
 * 最后一块内存的len设置为0，代表后面没有内存区域了
 */
void osal_mem_init(void) {
  OSAL_ASSERT(((OSALMEM_MIN_BLKSZ % OSALMEM_HDRSZ) == 0));
  OSAL_ASSERT(((OSALMEM_SMALL_BLKSZ % OSALMEM_HDRSZ) == 0));

#if OSALMEM_PROFILER
  (void)osal_memset(theHeap, OSALMEM_INIT, MAXMEMHEAP);
#endif

  // 最后一块内存的len设置为0，代表后面没有内存区域了
  theHeap[OSALMEM_LASTBLK_IDX].val = 0;

  // 小块内存管理区域的len设置为OSALMEM_SMALLBLK_BUCKET
  // Set 'len' & clear 'inUse' field.
  ff1 = theHeap;
  ff1->val = OSALMEM_SMALLBLK_BUCKET;

  // 中间一块内存设置为占用，用以分割两块区域
  // Set 'len' & 'inUse' fields - this is a 'zero data bytes' lifetime
  // allocation to block the small-block bucket from ever being coalesced with
  // the wilderness.
  theHeap[OSALMEM_SMALLBLK_HDRCNT].val = (OSALMEM_HDRSZ | OSALMEM_IN_USE);

  // 大块内存管理区域的len设置为OSALMEM_BIGBLK_SZ
  // Set 'len' & clear 'inUse' field.
  theHeap[OSALMEM_BIGBLK_IDX].val = OSALMEM_BIGBLK_SZ;

#if (OSALMEM_METRICS)
  /* Start with the small-block bucket and the wilderness - don't count the
   * end-of-heap NULL block nor the end-of-small-block NULL block.
   */
  blkCnt = blkFree = 2;
#endif
}

/*
 * 设置ff1指针跳过常驻内存区域，指向可申请区域的地址，加快后续的内存申请效率
 * 当系统任务都创建和初始化完成后调用此函数
 */
void osal_mem_kick(void) {
  osal_mem_hdr_t *tmp = osal_mem_alloc(1);
  OSAL_ASSERT((tmp != NULL));
  hal_reg_t cpu_sr = hal_enter_critical();

  // 此时申请的内存区域，已经在常驻内存区域的后面了
  // Set 'ff1' to point to the first available memory after the LL block.
  ff1 = tmp - 1;

  osal_mem_free(tmp);

  // Set 'mem_stat' after the free because it enables memory profiling.
  mem_stat = 0x01;
  hal_exit_critical(cpu_sr);
}

/**
 * @brief 申请内存
 *
 * @param size 期望申请的内存大小Byte
 * @return void* 成功返回申请到的内存地址，失败返回NULL
 */
#if DPRINTF_OSALHEAPTRACE
void *osal_mem_alloc_dbg(uint16_t size, const char *fname, unsigned lnum)
#else  /* DPRINTF_OSALHEAPTRACE */
void *osal_mem_alloc(uint16_t size)
#endif /* DPRINTF_OSALHEAPTRACE */
{
  osal_mem_hdr_t *prev = NULL;
  osal_mem_hdr_t *hdr;
  hal_reg_t intState;
  uint8_t coal = 0;

  size += OSALMEM_HDRSZ;

  // size字对齐
  // Calculate required bytes to add to 'size' to align to halDataAlign_t.
  if (sizeof(halDataAlign_t) == 2) {
    size += (size & 0x01);
  } else if (sizeof(halDataAlign_t) != 1) {
    const uint8_t mod = size % sizeof(halDataAlign_t);

    if (mod != 0) {
      size += (sizeof(halDataAlign_t) - mod);
    }
  }

  // HAL_ENTER_CRITICAL_SECTION(intState); // Hold off interrupts.
  intState = hal_enter_critical();

  // 初始化阶段只在固定区域进行分配，这些内存就是常驻内存了
  // 初始化完成后，如果申请的size小于OSALMEM_SMALL_BLKSZ，则直接从固定区域分配，否则从非固定区域分配
  // Smaller allocations are first attempted in the small-block bucket, and all
  // long-lived allocations are channeled into the LL block reserved within this
  // bucket.
  if ((mem_stat == 0) || (size <= OSALMEM_SMALL_BLKSZ)) {
    hdr = ff1;
  } else {
    hdr = (theHeap + OSALMEM_BIGBLK_IDX);
  }

  // 1、如果hdr指向的区域未被使用，且大小大于等于申请的size，跳出循环
  // 2、如果hdr指向的区域未被使用，且大小小于等于申请的size，hdr跳转到下一个区域
  // 2.1、如果下一个区域也没被占用，则把两个区域合并起来，再次判断大小是否OK，如果OK跳出循环，如果不OK，HDR继续跳到下一个区域
  // 2.2、如果下一个区域被占用，则跳转到下一个区域
  do {
    if (hdr->inUse) {
      coal = 0;
    } else {
      if (coal != 0) {
#if (OSALMEM_METRICS)
        blkCnt--;
        blkFree--;
#endif

        prev->len += hdr->len;

        if (prev->len >= size) {
          hdr = prev;
          break;
        }
      } else {
        if (hdr->len >= size) {
          break;
        }

        coal = 1;
        prev = hdr;
      }
    }

    hdr = (osal_mem_hdr_t *)((uint8_t *)hdr + hdr->len);

    if (hdr->val == 0) {
      hdr = NULL;
      break;
    }
  } while (1);

  // 如果找到了合适的区域，看看要不要进行拆分
  // 当区域的大小超过size+OSALMEM_MIN_BLKSZ，则进行拆分
  // 否则不进行拆分了
  if (hdr != NULL) {
    uint16_t tmp = hdr->len - size;

    // Determine whether the threshold for splitting is met.
    if (tmp >= OSALMEM_MIN_BLKSZ) {
      // Split the block before allocating it.
      osal_mem_hdr_t *next = (osal_mem_hdr_t *)((uint8_t *)hdr + size);
      next->val = tmp;                    // Set 'len' & clear 'inUse' field.
      hdr->val = (size | OSALMEM_IN_USE); // Set 'len' & 'inUse' field.

#if (OSALMEM_METRICS)
      blkCnt++;
      if (blkMax < blkCnt) {
        blkMax = blkCnt;
      }
      memAlo += size;
#endif
    } else {
#if (OSALMEM_METRICS)
      memAlo += hdr->hdr.len;
      blkFree--;
#endif

      hdr->inUse = TRUE;
    }

#if (OSALMEM_METRICS)
    if (memMax < memAlo) {
      memMax = memAlo;
    }
#endif

#if (OSALMEM_PROFILER)
#if !OSALMEM_PROFILER_LL
    if (mem_stat != 0) // Don't profile until after the LL block is filled.
#endif
    {
      uint8_t idx;

      for (idx = 0; idx < OSALMEM_PROMAX; idx++) {
        if (hdr->hdr.len <= proCnt[idx]) {
          break;
        }
      }
      proCur[idx]++;
      if (proMax[idx] < proCur[idx]) {
        proMax[idx] = proCur[idx];
      }
      proTot[idx]++;

      /* A small-block could not be allocated in the small-block bucket.
       * When this occurs significantly frequently, increase the size of the
       * bucket in order to restore better worst case run times. Set the first
       * profiling bucket size in proCnt[] to the small-block bucket size and
       * divide proSmallBlkMiss by the corresponding proTot[] size to get %
       * miss. Best worst case time on TrasmitApp was achieved at a 0-15% miss
       * rate during steady state Tx load, 0% during idle and steady state Rx
       * load.
       */
      if ((hdr->hdr.len <= OSALMEM_SMALL_BLKSZ) &&
          (hdr >= (theHeap + OSALMEM_BIGBLK_IDX))) {
        proSmallBlkMiss++;
      }
    }

    (void)osal_memset((uint8_t *)(hdr + 1), OSALMEM_ALOC,
                      (hdr->hdr.len - OSALMEM_HDRSZ));
#endif

    // 如果分配的区域是最开始的块，移动ff1，提高下次分配的效率
    if ((mem_stat != 0) && (ff1 == hdr)) {
      ff1 = (osal_mem_hdr_t *)((uint8_t *)hdr + hdr->len);
    }

    // 返回给调用者的是把头部去掉后的真正可用的区域
    hdr++;
  }

  // HAL_EXIT_CRITICAL_SECTION(intState); // Re-enable interrupts.
  hal_exit_critical(intState);

  OSAL_ASSERT(((size_t)hdr % sizeof(halDataAlign_t)) == 0);

#if DPRINTF_OSALHEAPTRACE
  dprintf("osal_mem_alloc(%u)->%lx:%s:%u\n", size, (unsigned)hdr, fname, lnum);
#endif /* DPRINTF_OSALHEAPTRACE */
  return (void *)hdr;
}

/*
 * Free a block of memory.
 */
#if DPRINTF_OSALHEAPTRACE
void osal_mem_free_dbg(void *ptr, const char *fname, unsigned lnum)
#else  /* DPRINTF_OSALHEAPTRACE */
void osal_mem_free(void *ptr)
#endif /* DPRINTF_OSALHEAPTRACE */
{
  osal_mem_hdr_t *hdr = (osal_mem_hdr_t *)ptr - 1;
  hal_reg_t intState;

#if DPRINTF_OSALHEAPTRACE
  dprintf("osal_mem_free(%lx):%s:%u\n", (unsigned)ptr, fname, lnum);
#endif /* DPRINTF_OSALHEAPTRACE */

  HAL_ASSERT(((uint8_t *)ptr >= (uint8_t *)theHeap) &&
             ((uint8_t *)ptr < (uint8_t *)theHeap + MAXMEMHEAP));
  HAL_ASSERT(hdr->inUse);

  // HAL_ENTER_CRITICAL_SECTION(intState); // Hold off interrupts.
  intState = hal_enter_critical();
  hdr->inUse = FALSE;

  // 释放的区域在ff1前面，移动ff1
  if (ff1 > hdr) {
    ff1 = hdr;
  }

#if OSALMEM_PROFILER
#if !OSALMEM_PROFILER_LL
  if (mem_stat != 0) // Don't profile until after the LL block is filled.
#endif
  {
    uint8_t idx;

    for (idx = 0; idx < OSALMEM_PROMAX; idx++) {
      if (hdr->hdr.len <= proCnt[idx]) {
        break;
      }
    }

    proCur[idx]--;
  }

  (void)osal_memset((uint8_t *)(hdr + 1), OSALMEM_REIN,
                    (hdr->hdr.len - OSALMEM_HDRSZ));
#endif
#if OSALMEM_METRICS
  memAlo -= hdr->hdr.len;
  blkFree++;
#endif

  // HAL_EXIT_CRITICAL_SECTION(intState); // Re-enable interrupts.
  hal_exit_critical(intState);
}

#if OSALMEM_METRICS
/*********************************************************************
 * @fn      osal_heap_block_max
 *
 * @brief   Return the maximum number of blocks ever allocated at once.
 *
 * @param   none
 *
 * @return  Maximum number of blocks ever allocated at once.
 */
uint16_t osal_heap_block_max(void) { return blkMax; }

/*********************************************************************
 * @fn      osal_heap_block_cnt
 *
 * @brief   Return the current number of blocks now allocated.
 *
 * @param   none
 *
 * @return  Current number of blocks now allocated.
 */
uint16_t osal_heap_block_cnt(void) { return blkCnt; }

/*********************************************************************
 * @fn      osal_heap_block_free
 *
 * @brief   Return the current number of free blocks.
 *
 * @param   none
 *
 * @return  Current number of free blocks.
 */
uint16_t osal_heap_block_free(void) { return blkFree; }

/*********************************************************************
 * @fn      osal_heap_mem_used
 *
 * @brief   Return the current number of bytes allocated.
 *
 * @param   none
 *
 * @return  Current number of bytes allocated.
 */
uint16_t osal_heap_mem_used(void) { return memAlo; }

/*********************************************************************
 * @fn      osal_heap_high_water
 *
 * @brief   Return the highest byte ever allocated in the heap.
 *
 * @param   none
 *
 * @return  Highest number of bytes ever used by the stack.
 */
uint16_t osal_heap_high_water(void) {
#if (OSALMEM_METRICS)
  return memMax;
#else
  return MAXMEMHEAP;
#endif
}
#endif

#ifdef __cplusplus
}
#endif