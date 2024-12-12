/**
 * @file hal_tick.c
 * @author ljgabc
 * @brief Linux平台下tick实现，在一个线程中定时调用
 * @version 0.1
 * @date 2024-11-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "osal.h"
#include "n32g430.h"

/**
 * @brief 每次滴答的时间ms
 * 
 */
#define HAL_TICK_PERIOD_MS 10

/**
 * @brief SysTick中断频率
 * 
 */
 */
#define SYSTICK_FREQ (1000UL / HAL_TICK_PERIOD_MS)

/**
 * @brief 滴答中断
 * 
 */
void SysTick_Handler(void)
{
  osal_tick(HAL_TICK_PERIOD_MS);
}

/**
 * @brief 定时器初始化，设定系统时钟
 */
void hal_tick_init(void) {
  SysTick_Config(SystemCoreClockFrequency / SYSTICK_FREQ);
}

/**
 * @brief
 * 开启tick，OSAL会根据程序中软件定时器的实际使用动态开启和关闭，为空则一直开启
 */
void hal_tick_start(void) {}

/**
 * @brief 关闭tick，为空则一直不关闭
 */
void hal_tick_stop(void) {}
