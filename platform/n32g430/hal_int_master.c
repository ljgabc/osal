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
#include "n32g430.h"

/**
 * @brief 使能全局中断
 *
 */
void hal_enable_interrupt(void)
{
    __enable_irq();
}

/**
 * @brief 禁用全局中断
 *
 */
void hal_disable_interrupt(void
{
    __disable_irq();
}

/**
 * @brief 禁用全局中断，并保存当前中断使能状态
 *
 * @return hal_reg_t
 */
hal_reg_t hal_enter_critical(void)
{
    return __get_CONTROL();
}

/**
 * @brief 根据cpu_sr的值，恢复中断使能状态
 *
 * @param cpu_sr
 */
void hal_exit_critical(hal_reg_t cpu_sr)
{
    __set_CONTROL(cpu_sr);
}
