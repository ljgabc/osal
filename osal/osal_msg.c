/**
 * @file osal_msg.c
 * @author ljgabc
 * @brief 消息处理
 * @version 0.1
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "osal.h"

#define SYS_EVENT_MSG 0x8000

#define OSAL_MSG_NEXT(msg_ptr) ((struct osal_msg_hdr *)(msg_ptr) - 1)->next
#define OSAL_MSG_TASK(msg_ptr) ((struct osal_msg_hdr *)(msg_ptr) - 1)->task

struct osal_msg_hdr {
  struct osal_msg_hdr *next;
  struct osal_tcb *task;
  uint16_t len;
};

static struct osal_msg_hdr *osal_qHead;

// 消息队列头指针
static void *msg_queue_head = NULL;

// 消息队列尾指针
static void *msg_queue_tail = NULL;

// typedef struct {
//   uint8_t event;
//   uint8_t status;
// } osal_event_hdr_t;

// // 默认系统消息结构体
// typedef struct {
//   osal_event_hdr_t hdr;
//   uint8_t *Data_t;
// } osal_sys_msg_t;

/**
 * @brief 任务调用此函数来分配消息缓冲区
 *
 * @param len 所需缓冲区长度(不含消息头)
 * @return uint8_t*
 * 指向已分配缓冲区的指针，只包含消息本体，可以直接使用；失败返回NULL
 */
uint8_t *osal_msg_allocate(uint16_t len) {
  struct osal_msg_hdr *hdr;

  if (len == 0) {
    return (NULL);
  }

  hdr = (struct osal_msg_hdr *)osal_mem_alloc(
      (uint16_t)(len + sizeof(struct osal_msg_hdr)));
  if (hdr) {
    hdr->next = NULL;
    hdr->task = NULL;
    hdr->len = len;
    return ((uint8_t *)(hdr + 1)); // 返回消息本体的指针（跳过消息头）
  } else {
    return (NULL);
  }
}

/**
 * @brief 任务完成对收到的消息处理后，调用此函数释放缓冲区
 * @param msg_ptr 指向缓冲区的指针
 *
 * @return uint8_t 成功返回OK
 */
uint8_t osal_msg_deallocate(uint8_t *msg_ptr) {
  if (msg_ptr == NULL) {
    return (OSAL_INVALID_MSG_POINTER);
  }

  // 如果消息在队列中，返回错误
  if ((OSAL_MSG_TASK(msg_ptr) != NULL) || (OSAL_MSG_NEXT(msg_ptr) != NULL)) {
    return (OSAL_MSG_BUFFER_NOT_AVAIL);
  }

  uint8_t *ptr = (uint8_t *)((uint8_t *)msg_ptr - sizeof(struct osal_msg_hdr));

  osal_mem_free((void *)ptr);

  return (OSAL_OK);
}

/**
 * @brief
 * 任务调用此函数向其他任务发送命令或消息，消息的释放由接收方负责，发送方不用管。
 *
 * @param task 目标任务
 * @param msg_ptr 通过osal_msg_allocate申请到的消息缓冲区指针
 * @return uint8_t 成功返回OK
 */
uint8_t osal_msg_send(const struct osal_tcb *task, uint8_t *msg_ptr) {
  if (msg_ptr == NULL) {
    return (OSAL_INVALID_MSG_POINTER);
  }

  // 发送方不负责消息缓冲区的释放，如果task不存在，需要释放缓冲区
  if (task == NULL) {
    osal_msg_deallocate(msg_ptr);
    return (OSAL_INVALID_TASK);
  }

  // 将消息入队
  OSAL_MSG_TASK(msg_ptr) = task;
  osal_msg_enqueue(msg_ptr);

  // 通知任务有消息
  osal_set_event(task, SYS_EVENT_MSG);

  return (OSAL_OK);
}

/**
 * @brief 任务调用此函数接受和处理消息
 * 在处理完消息后，任务需要调用osal_msg_deallocate来释放消息缓冲区
 *
 * @param task_id
 * @return uint8_t*
 */
uint8_t *osal_msg_receive(const struct osal_tcb *task) {
  struct osal_msg_hdr *listHdr;
  struct osal_msg_hdr *prevHdr = NULL;
  struct osal_msg_hdr *foundHdr = NULL;

  // Hold off interrupts
  hal_reg_t cpu_sr = hal_enter_critical();

  // Point to the top of the queue
  listHdr = osal_qHead;

  // Look through the queue for a message that belongs to the asking task
  while (listHdr != NULL) {
    if ((listHdr - 1)->task == task) {
      if (foundHdr == NULL) {
        // Save the first one
        foundHdr = listHdr;
      } else {
        // Second msg found, stop looking
        break;
      }
    }
    if (foundHdr == NULL) {
      prevHdr = listHdr;
    }
    listHdr = OSAL_MSG_NEXT(listHdr);
  }

  // Is there more than one?
  if (listHdr != NULL) {
    // Yes, Signal the task that a message is waiting
    osal_set_event(task, SYS_EVENT_MSG);
  } else {
    // No more
    osal_clear_event(task, SYS_EVENT_MSG);
  }

  // Did we find a message?
  if (foundHdr != NULL) {
    // Take out of the link list
    osal_msg_extract(&osal_qHead, foundHdr, prevHdr);
  }

  // Release interrupts
  hal_exit_critical(cpu_sr);

  return ((uint8_t *)foundHdr);
}

/**
 * @brief 查找与指定task_id和event参数匹配的消息
 *
 * @param task_id 目标任务
 * @param event 目标事件
 * @return osal_event_hdr_t* 消息头
 */
osal_event_hdr_t *osal_msg_find(const struct osal_tcb *task, uint8_t event) {
  struct osal_msg_hdr *pHdr;

  // Hold off interrupts.
  hal_reg_t cpu_sr = hal_enter_critical();

  pHdr = osal_qHead; // Point to the top of the queue.

  // Look through the queue for a message that matches the task_id and event
  // parameters.
  while (pHdr != NULL) {
    if (((pHdr - 1)->task == task) &&
        (((osal_event_hdr_t *)pHdr)->event == event)) {
      break;
    }

    pHdr = OSAL_MSG_NEXT(pHdr);
  }

  hal_exit_critical(cpu_sr); // Release interrupts.

  return (osal_event_hdr_t *)pHdr;
}

/**
 * @brief 强制将一个OSAL消息入队到OSAL队列中
 *
 * @param q_ptr 队列指针
 * @param msg_ptr 消息指针
 */
void osal_msg_enqueue(void *msg_ptr) {

  hal_reg_t cpu_sr = hal_enter_critical();

  OSAL_MSG_NEXT(msg_ptr) = NULL;

  // If first message in queue
  if (msg_queue_head == NULL) {
    msg_queue_head = msg_ptr;
  } else {
    // Add message to end of queue
    void *ptr = msg_queue_head;
    while (OSAL_MSG_NEXT(ptr) != NULL) {
      ptr = OSAL_MSG_NEXT(ptr);
    }
    OSAL_MSG_NEXT(ptr) = msg_ptr;
  }
  hal_exit_critical(cpu_sr);
}

/**
 * @brief 从消息队列中取出一条消息
 *
 * @param q_ptr 队列指针
 * @return void* 成功返回消息指针，队列为空返回NULL
 */
void *osal_msg_dequeue() {
  void *msg_ptr = NULL;

  hal_reg_t cpu_sr = hal_enter_critical();

  if (msg_queue_head != NULL) {
    // Dequeue message
    msg_ptr = msg_queue_head;
    OSAL_MSG_NEXT(msg_ptr) = NULL;
    msg_queue_head = OSAL_MSG_NEXT(msg_ptr);
  }

  hal_exit_critical(cpu_sr);

  return msg_ptr;
}

/**
 * @brief 将消息添加到队列头
 *
 * @param q_ptr 队列指针
 * @param msg_ptr 消息指针
 */
void osal_msg_push(void *msg_ptr) {
  hal_reg_t cpu_sr = hal_enter_critical();

  // Push message to head of queue
  OSAL_MSG_NEXT(msg_ptr) = msg_queue_head;
  msg_queue_head = msg_ptr;

  hal_exit_critical(cpu_sr);
}

/**
 * @brief 从队列中移除一个消息
 *
 * @param q_ptr 队列指针
 * @param msg_ptr 要取出的消息指针
 * @param prev_ptr msg_ptr的前一个节点指针
 */
void osal_msg_extract(void *msg_ptr, void *prev_ptr) {
  hal_reg_t cpu_sr = hal_enter_critical();

  if (msg_ptr == msg_queue_head) {
    // remove from first
    msg_queue_head = OSAL_MSG_NEXT(msg_ptr);
  } else {
    // remove from middle
    OSAL_MSG_NEXT(prev_ptr) = OSAL_MSG_NEXT(msg_ptr);
  }
  OSAL_MSG_NEXT(msg_ptr) = NULL;

  // Re-enable interrupts
  hal_exit_critical(cpu_sr);
}

/**
 * @brief 尝试将一个OSAL消息入队到OSAL队列中，如果队列满了，则放弃
 *
 * @param q_ptr 队列指针
 * @param msg_ptr 消息指针
 * @param max 队列长度
 * @return uint8_t 成功入队返回OK
 */
uint8_t osal_msg_enqueue_max(void *msg_ptr, uint8_t max) {
  void *list;
  uint8_t ret = FALSE;

  // Hold off interrupts
  hal_reg_t cpu_sr = hal_enter_critical();

  // If first message in queue
  if (msg_queue_head == NULL) {
    msg_queue_head = msg_ptr;
    ret = TRUE;
  } else {
    // Find end of queue or max
    list = msg_queue_head;
    max--;
    while ((OSAL_MSG_NEXT(list) != NULL) && (max > 0)) {
      list = OSAL_MSG_NEXT(list);
      max--;
    }

    // Add message to end of queue if max not reached
    if (max != 0) {
      OSAL_MSG_NEXT(list) = msg_ptr;
      ret = TRUE;
    }
  }

  // Re-enable interrupts
  hal_exit_critical(cpu_sr);

  return ret;
}
