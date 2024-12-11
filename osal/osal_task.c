/**
 * @file osal_task.c
 * @author ljgabc
 * @brief 任务和事件管理
 * @version 0.1
 * @date 2024-11-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "osal.h"

/**
 * @brief 任务控制块实现
 */
struct osal_tcb {
  struct osal_tcb *next;
  task_init_fn_t init;       // 任务初始化函数指针
  task_handler_fn_t handler; // 任务事件处理函数指针
  uint8_t id;                // 任务ID
  uint8_t priority;          // 任务优先级
  uint16_t events;           // 任务事件
};

// 任务链表表头
static struct osal_tcb *task_list_head = NULL;

// 任务总数
static uint8_t total_task_cnt = 0;

/**
 * @brief 初始化任务列表
 *
 */
void osal_task_init(void) {
  task_list_head = (struct osal_tcb *)NULL;
  total_task_cnt = 0;
}

/**
 * @brief 设置任务的事件标志，将event_flag与任务的events进行或运算
 *
 * @param task_id 任务id
 * @param event_flag 期望设置的事件
 * @return int8 成功返回0
 */
int8_t osal_set_event(struct osal_tcb *task, uint16_t event_flag) {
  if (task) {
    hal_reg_t cpu_sr = hal_enter_critical();
    task->events |= event_flag;
    hal_exit_critical(cpu_sr);
    return (OSAL_OK);
  }
  return (OSAL_INVALID_TASK);
}

/**
 * @brief 清除任务的事件标志
 *
 * @param task_id 任务id
 * @param event_flag 期望清除的事件
 * @return int8 成功返回0
 */
int8_t osal_clear_event(struct osal_tcb *task, uint16_t event_flag) {
  if (task) {
    hal_reg_t cpu_sr = hal_enter_critical();
    task->events &= ~event_flag;
    hal_exit_critical(cpu_sr);
    return (OSAL_OK);
  }
  return (OSAL_INVALID_TASK);
}

/**
 * @brief 获取任务事件标志
 *
 * @param task 任务

 * @return uint16_t 事件标志
 */
uint16_t osal_get_event(const struct osal_tcb *task) {
  uint16_t event_flag = 0;
  if (task) {
    hal_reg_t cpu_sr = hal_enter_critical();
    event_flag = task->events;
    hal_exit_critical(cpu_sr);
  }
  return event_flag;
}

/**
 * @brief 调用所有任务的初始化函数
 *
 */
void osal_task_runinit(void) {
  for (struct osal_tcb *task = task_list_head; task != NULL;
       task = task->next) {
    if (task->init) {
      task->init(task);
    }
  }
}

/**
 * @brief 查找最高优先级的有效任务并执行事件处理
 *
 */
void osal_task_polling(void) {

  // 获取就绪的任务
  struct osal_tcb *task = osal_next_active_task();

  if (task) {

    // 暂存任务事件标志并清零
    hal_reg_t cpu_sr = hal_enter_critical();
    uint16_t events = task->events;
    task->events = 0;
    hal_exit_critical(cpu_sr);

    // 执行任务处理函数，返回需要再次置位的事件标志
    if (events != 0 && task->handler) {
      events = (task->handler)(task, events);
      osal_set_event(task, events);
    }
  }
}

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
                      uint8_t priority) {

  // 超过最大任务数量
  if (total_task_cnt >= OSAL_MAX_TASKS) {
    return OSAL_INVALID_TASK_ID;
  }

  struct osal_tcb *task_new = osal_mem_alloc(sizeof(struct osal_tcb));

  if (task_new) {
    task_new->init = init;
    task_new->handler = handler;
    task_new->events = 0;
    task_new->priority = priority;
    task_new->next = (struct osal_tcb *)NULL;
    hal_reg_t cpu_sr = hal_enter_critical();
    task_new->id = total_task_cnt;
    total_task_cnt++; // 任务数量统计
    hal_exit_critical(cpu_sr);

    struct osal_tcb **prev_task_ptr = &task_list_head;
    for (struct osal_tcb *task = task_list_head; task != NULL;
         task = task->next) {
      if (task_new->priority > task->priority) {
        task_new->next = task;
        *prev_task_ptr = task_new;
        return task_new->id;
      }
      prev_task_ptr = &task->next;
    }
    *prev_task_ptr = task_new;
  }
  return OSAL_INVALID_TASK_ID;
}

/**
 * @brief 获取最高优先级的就绪任务的任务控制块
 *
 * @return struct osal_tcb* 最高优先级的就绪任务
 */
struct osal_tcb *osal_next_active_task(void) {
  for (struct osal_tcb *task = task_list_head; task != NULL;
       task = task->next) {
    if (task->events) {
      return task;
    }
  }
  return ((struct osal_tcb *)NULL);
}
