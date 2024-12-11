/**
 * @file osal.c
 * @author ljgabc (ljgabc@gmail.com)
 * @brief OSAL抽象层接口
 * @version 0.1
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "osal.h"
#include <string.h>

osal_msg_q_t osal_qHead;

/**
 * @brief 初始化系统，如线程表、内存管理系统的等
 *
 * @return uint8
 */
uint8_t osal_init(void) {
  // 初始化动态内存分配器
  osal_mem_init();

  // 初始化消息队列
  osal_qHead = NULL;

#if defined(OSAL_TOTAL_MEM)
  osal_msg_cnt = 0;
#endif

  // 初始化时钟
  osal_timer_init();

  // 初始化任务列表
  osal_task_init();

  return (ZSUCCESS);
}

/**
 * @brief 运行osal系统，此函数不会返回
 * 需要在运行之前，创建好所有的任务
 *
 */
void osal_run(void) {
  // 调用任务的初始化函数
  osal_task_runinit();

  while (1) {

    // 运行任务
    osal_task_polling();
  }
}

#if 0
/*********************************************************************
 * @fn osal_strlen
 *
 * @brief
 *
 *   Calculates the length of a string.  The string must be null
 *   terminated.
 *
 * @param   char *pString - pointer to text string
 *
 * @return  int - number of characters
 */
int osal_strlen(char *pString) { return (int)(strlen(pString)); }

/*********************************************************************
 * @fn osal_memcpy
 *
 * @brief
 *
 *   Generic memory copy.
 *
 *   Note: This function differs from the standard memcpy(), since
 *         it returns the pointer to the next destination uint8. The
 *         standard memcpy() returns the original destination address.
 *
 * @param   dst - destination address
 * @param   src - source address
 * @param   len - number of bytes to copy
 *
 * @return  pointer to end of destination buffer
 */
void *osal_memcpy(void *dst, const void *src, unsigned int len) {
  uint8 *pDst;
  const uint8 *pSrc;

  pSrc = src;
  pDst = dst;

  while (len--)
    *pDst++ = *pSrc++;

  return (pDst);
}

/*********************************************************************
 * @fn osal_revmemcpy
 *
 * @brief   Generic reverse memory copy.  Starts at the end of the
 *   source buffer, by taking the source address pointer and moving
 *   pointer ahead "len" bytes, then decrementing the pointer.
 *
 *   Note: This function differs from the standard memcpy(), since
 *         it returns the pointer to the next destination uint8. The
 *         standard memcpy() returns the original destination address.
 *
 * @param   dst - destination address
 * @param   src - source address
 * @param   len - number of bytes to copy
 *
 * @return  pointer to end of destination buffer
 */
void *osal_revmemcpy(void *dst, const void *src, unsigned int len) {
  uint8 *pDst;
  const uint8 *pSrc;

  pSrc = src;
  pSrc += (len - 1);
  pDst = dst;

  while (len--)
    *pDst++ = *pSrc--;

  return (pDst);
}

/*********************************************************************
 * @fn osal_memdup
 *
 * @brief   Allocates a buffer [with osal_mem_alloc()] and copies
 *          the src buffer into the newly allocated space.
 *
 * @param   src - source address
 * @param   len - number of bytes to copy
 *
 * @return  pointer to the new allocated buffer, or NULL if
 *          allocation problem.
 */
void *osal_memdup(const void *src, unsigned int len) {
  uint8 *pDst;

  pDst = osal_mem_alloc(len);
  if (pDst) {
    osal_memcpy(pDst, src, len);
  }

  return ((void *)pDst);
}

/*********************************************************************
 * @fn osal_memcmp
 *
 * @brief
 *
 *   Generic memory compare.
 *
 * @param   src1 - source 1 addrexx
 * @param   src2 - source 2 address
 * @param   len - number of bytes to compare
 *
 * @return  TRUE - same, FALSE - different
 */
uint8 osal_memcmp(const void *src1, const void *src2, unsigned int len) {
  const uint8 *pSrc1;
  const uint8 *pSrc2;

  pSrc1 = src1;
  pSrc2 = src2;

  while (len--) {
    if (*pSrc1++ != *pSrc2++)
      return FALSE;
  }
  return TRUE;
}

/*********************************************************************
 * @fn osal_memset
 *
 * @brief
 *
 *   Set memory buffer to value.
 *
 * @param   dest - pointer to buffer
 * @param   value - what to set each uint8 of the message
 * @param   size - how big
 *
 * @return  pointer to destination buffer
 */
void *osal_memset(void *dest, uint8 value, int len) {
  return memset(dest, value, len);
}

#endif