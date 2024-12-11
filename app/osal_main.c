/**
 * @file osal_main.c
 * @brief osal操作系统运行主函数，添加任务在此文件中添加
 * @version 0.1
 * @date 2019-07-25
 * @author WatWu
 */

#include "task_event.h"

void osal_main(void)
{
    //系统硬件、外设等初始化

    //禁止中断
    HAL_DISABLE_INTERRUPTS();

    //osal操作系统初始化
    osal_init();

    //添加任务
    osal_add_Task(print_task_init, print_task_event_process, 1);
    osal_add_Task(statistics_task_init, statistics_task_event_process, 2);

    

    osal_mem_kick();

    //允许中断
    HAL_ENABLE_INTERRUPTS();

    //设置初始任务事件，上电就需要自动轮询的任务事件可在此添加

    //启动osal系统，不会再返回
    osal_run();
}
