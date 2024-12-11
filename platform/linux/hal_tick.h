/**
 * @file hal_timer.h
 * @author ljgabc
 * @brief 硬件定时器实现，为osal操作系统提供系统滴答心跳时钟
 * 每次系统滴答心跳时调用一次osal_update_timers()
 * @version 0.1
 * @date 2024-11-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

/**
 * @brief 每次滴答的时间ms
 * 
 */
#define HAL_TICK_PERIOD_MS 10

