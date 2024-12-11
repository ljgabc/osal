/**
 * @file hal_int_master.c
 * @author ljgabc
 * @brief 临界区控制
 * @version 0.1
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "osal.h"

// #define HAL_ENABLE_INTERRUPTS()         SEI()       // Enable Interrupts
// #define HAL_DISABLE_INTERRUPTS()        CLI()       // Disable Interrupts
// #define HAL_INTERRUPTS_ARE_ENABLED()    SEI()       // Enable Interrupts

// #define HAL_ENTER_CRITICAL_SECTION()    CLI()
// #define HAL_EXIT_CRITICAL_SECTION()     SEI()

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
