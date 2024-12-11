/**
 * @file osal_msg.h
 * @author ljgabc
 * @brief 消息处理
 * @version 0.1
 * @date 2024-11-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "osal_types.h"
#include "osal_task.h"






// typedef void *osal_msg_q_t;

/**
 * @brief 任务调用此函数来分配消息缓冲区
 *
 * @param len 所需缓冲区长度
 * @return uint8_t* 指向已分配缓冲区的指针，如果分配失败，则返回 NULL
 */
uint8_t *osal_msg_allocate(uint16_t len);

/**
 * @brief 释放消息缓冲区
 * 当任务完成对收到的消息处理后调用此函数。
 * @param msg_ptr 指向缓冲区的指针
 * @return uint8_t 成功返回OK，缓冲区错误返回INVALID_MSG_POINTER
 */
uint8_t osal_msg_deallocate(uint8_t *msg_ptr);

/**
 * @brief 强制将一个OSAL消息入队到OSAL队列中
 *
 * @param msg_ptr 消息指针
 */
void osal_msg_enqueue(void *msg_ptr);

/**
 * @brief 尝试将一个OSAL消息入队到OSAL队列中，如果队列满了，则放弃
 *
 * @param msg_ptr 消息指针
 * @param max 队列长度
 * @return uint8_t 成功入队返回OK
 */
uint8_t osal_msg_enqueue_max(void *msg_ptr, uint8_t max);

/**
 * @brief 从消息队列中取出一条消息
 *
 * @return void* 成功返回消息指针，队列为空返回NULL
 */
void *osal_msg_dequeue(void);

/**
 * @brief 将消息添加到队列头
 *
 * @param q_ptr 队列指针
 * @param msg_ptr 消息指针
 */
void osal_msg_push(void *msg_ptr);

/**
 * @brief 从队列中取出并移除一个消息
 *
 * @param msg_ptr 要取出的消息指针
 * @param prev_ptr msg_ptr的前一个节点指针
 */
void osal_msg_extract(void *msg_ptr, void *prev_ptr);

/**
 * @brief 任务调用此函数向其他任务发送命令或消息
 *
 * @param task_id 目标任务
 * @param msg_ptr 消息缓冲区指针
 * @return uint8_t 成功返回OK
 */
uint8_t osal_msg_send(const struct osal_tcb* task, uint8_t *msg_ptr);

/**
 * @brief
 * 任务调用此函数接受消息，在处理完消息后，任务需要调用osal_msg_deallocate来释放消息缓冲区
 *
 * @param task_id
 * @return uint8_t*
 */
uint8_t *osal_msg_receive(const struct osal_tcb* task);

/**
 * @brief 查找与指定task_id和event 参数匹配的消息
 *
 * @param task_id 目标任务
 * @param event 目标事件
 * @return osal_event_hdr_t* 消息头
 */
osal_event_hdr_t *osal_msg_find(const struct osal_tcb* task, uint8_t event);