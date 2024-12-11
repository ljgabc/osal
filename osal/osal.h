/**
 * @file osal.h
 * @author ljgabc
 * @brief OS抽象层接口
 * @version 0.1
 * @date 2024-11-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "hal_types.h"
#include "osal_config.h"
#include "osal_memory.h"
#include "osal_msg.h"
#include "osal_port.h"
#include "osal_task.h"
#include "osal_timer.h"
#include "osal_types.h"

// extern osal_msg_q_t osal_qHead;
extern uint8_t total_task_cnt; // 任务数量统计

/**
 * @brief 初始化系统，如线程表、内存管理系统的等
 *
 * @return uint8
 */
uint8_t osal_init(void);

/**
 * @brief 运行osal系统，执行任务
 * 此函数不会返回
 *
 */
void osal_run(void);

// int osal_strlen(char *pString);
// void *osal_memcpy(void *dst, const void *src, unsigned int len);
// void *osal_revmemcpy(void *dst, const void *src, unsigned int len);
// void *osal_memdup(const void *src, unsigned int len);
// uint8 osal_memcmp(const void *src1, const void *src2, unsigned int len);
// void *osal_memset(void *dest, uint8 value, int len);
