/**
 * @file osal_task.h
 * @author ljgabc
 * @brief 任务和事件管理
 * @version 0.1
 * @date 2024-11-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "osal_types.h"

// 虚拟任务控制块
struct osal_tcb;

// 任务初始化函数
typedef void (*task_init_fn_t)(struct osal_tcb *task);

// 任务事件处理函数
typedef uint16_t (*task_handler_fn_t)(struct osal_tcb *task, uint16_t event);

/**
 * @brief 添加一个任务
 *
 * @param init   任务初始化函数
 * @param handler   任务事件处理函数
 * @param task_priority 任务优先级
 *
 * @return uint8_t 任务id
 */
uint8_t osal_add_task(task_init_fn_t init, task_handler_fn_t handler,
                      uint8_t priority);

/**
 * @brief 初始化任务列表
 *
 */
void osal_task_init(void);

/**
 * @brief 调用所有任务的初始化函数
 *
 */
void osal_task_runinit(void);

/**
 * @brief 获取最高优先级的有效任务并执行
 *
 */
void osal_task_polling(void);

/**
 * @brief 获取最高优先级的就绪任务
 *
 * @return struct osal_tcb*
 */
struct osal_tcb *osal_next_active_task(void);

/**
 * @brief 设置任务的事件标志，将event_flag与任务的events进行或运算
 *
 * @param task_id 任务id
 * @param event_flag 期望设置的事件
 * @return int8 成功返回0
 */
int8_t osal_set_event(struct osal_tcb *task, uint16_t event_flag);

/**
 * @brief 清除任务的事件标志
 *
 * @param task_id 任务id
 * @param event_flag 期望清除的事件
 * @return int8 成功返回0
 */
int8_t osal_clear_event(struct osal_tcb *task, uint16_t event_flag);

/**
 * @brief 获取任务事件标志
 *
 * @param task 任务

 * @return uint16_t 事件标志
 */
uint16_t osal_get_event(const struct osal_tcb *task);