#pragma once

#include "osal_types.h"
#include "osal_task.h"

#define TIMER_DECR_TIME 1 // 任务定时器更新时自减的数值单位

/**
 * @brief 定时器模块初始化
 *
 */
void osal_timer_init(void);

/**
 * @brief 创建并启动一个定时器
 *
 * @param task_id 任务ID
 * @param event_id 事件ID
 * @param timeout 超时时间
 * @param oneshot 是否是单次定时器
 *
 * @return uint8_t 成功返回OK
 */
uint8_t osal_start_timer(const struct osal_tcb* task, uint16_t event_id, uint16_t timeout,
                         bool oneshot);

/**
 * @brief 暂停定时器
 *
 * @param task_id 任务ID
 * @param event_id 事件ID
 * @return uint8_t 成功返回OK
 */
uint8_t osal_stop_timer(const struct osal_tcb* task, uint16_t event_id);

/**
 * @brief 获取定时器的timeout
 *
 * @param task_id 任务ID
 * @param event_id 事件ID
 * @return uint16_t 超时时间
 */
uint16_t osal_timer_get_timeout(const struct osal_tcb* task, uint16_t event_id);

/**
 * @brief 当前活跃定时器数量
 *
 * @return uint8_t 活跃定时器数量
 */
uint8_t osal_timer_num_active(void);

/**
 * @brief 获取系统时间
 *
 * @return uint32_t 系统启动之后的的毫秒数
 */
uint32_t osal_millis(void);

/**
 * @brief 更新系统时间，应该在tick中断中调用
 *
 * @param ms 时间，单位ms
 */
void osal_tick(uint16_t ms);
