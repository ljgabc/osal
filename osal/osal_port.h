/**
 * @file osal_port.h
 * @author ljgabc
 * @brief OSAL移植需要实现的函数，另外在移植的时候需要在tick中断中调用osal_tick函数
 * @version 0.1
 * @date 2024-11-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "osal_types.h"

/**
 * @brief 使能全局中断
 *
 */
void hal_enable_interrupt(void);

/**
 * @brief 禁用全局中断
 *
 */
void hal_disable_interrupt(void);

/**
 * @brief 查询中断是否使能
 *
 */
bool hal_interrupt_enabled(void);

/**
 * @brief 禁用全局中断，并保存当前中断使能状态
 *
 * @return hal_reg_t
 */
hal_reg_t hal_enter_critical(void);

/**
 * @brief 根据cpu_sr的值，恢复中断使能状态
 *
 * @param cpu_sr
 */
void hal_exit_critical(hal_reg_t cpu_sr);

/**
 * @brief tick初始化，设定系统时钟
 *
 */
void hal_tick_init(void);

/**
 * @brief 启动tick时钟
 *
 */
void hal_tick_start(void);

/**
 * @brief 关闭tick时钟
 *
 */
void hal_tick_stop(void);

