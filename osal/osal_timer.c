#include "osal.h"

struct osal_timer {
  struct osal_timer *next;
  uint16_t timeout;      // 定时时间，每过一个系统时钟会自减
  uint16_t event_flag;   // 定时事件，定时时间减完产生任务事件
  uint16_t reload;       // 重装定时时间
  struct osal_tcb *task; // 响应的任务ID
};

static uint32_t osal_current_time;         // 记录系统时钟
static struct osal_timer *timer_list_head; // 任务定时器链表头指针
static uint8_t total_timer_cnt = 0;        // 定时器总数

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */
struct osal_timer *osal_add_timer(const struct osal_tcb *task,
                                  uint16_t event_flag, uint16_t timeout);
struct osal_timer *osal_find_timer(const struct osal_tcb *task,
                                   uint16_t event_flag);
void osal_delete_timer(struct osal_timer *rmTimer);

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/

/**
 * @brief 定时器模块初始化
 *
 */
void osal_timer_init(void) {
  timer_list_head = NULL;
  osal_current_time = 0;
  total_timer_cnt = 0;

  // 初始化硬件定时器
  hal_tick_init();

  // 启动硬件定时器
  hal_tick_start();
}

/*********************************************************************
 * @fn osal_add_timer
 *
 * @brief   Add a timer to the timer list.
 *          Ints must be disabled.
 *
 * @param   task_id
 * @param   event_flag
 * @param   timeout
 *
 * @return  struct osal_timer * - pointer to newly created timer
 */
struct osal_timer *osal_add_timer(const struct osal_tcb *task,
                                  uint16_t event_flag, uint16_t timeout) {
  // Look for an existing timer first
  struct osal_timer *timer_new = osal_find_timer(task, event_flag);

  // 定时器已经存在，更新timeout值
  if (timer_new) {
    timer_new->timeout = timeout;
    return (timer_new);
  }

  // 新建定时器
  timer_new = osal_mem_alloc(sizeof(struct osal_timer));

  if (timer_new) {
    timer_new->task = task;
    timer_new->event_flag = event_flag;
    timer_new->timeout = timeout;
    timer_new->next = NULL;
    timer_new->reload = 0;
    total_timer_cnt++;

    if (timer_list_head == NULL) {
      timer_list_head = timer_new;
    } else {
      // Add it to the end of the timer list

      struct osal_timer *timer_ptr = timer_list_head;
      // Stop at the last record
      while (timer_ptr->next) {
        timer_ptr = timer_ptr->next;
      }
      timer_ptr->next = timer_new;
    }
    return (timer_new);
  } else {
    return ((struct osal_timer *)NULL);
  }
}

/**
 * @brief 查找指定定时器
 *
 * @param task_id
 * @param event_flag
 * @return struct osal_timer*
 */
struct osal_timer *osal_find_timer(const struct osal_tcb *task,
                                   uint16_t event_flag) {
  struct osal_timer *timer_ptr = timer_list_head;

  for (; timer_ptr != NULL; timer_ptr = timer_ptr->next) {
    if (timer_ptr->event_flag == event_flag && timer_ptr->task == task)
      break;
  }

  return (timer_ptr);
}

/**
 * @brief 删除一个定时器
 *
 * @param rmTimer 定时器指针
 */
void osal_delete_timer(struct osal_timer *timer) {
  if (timer) {
    // Clear the event flag and osalTimerUpdate() will delete
    // the timer from the list.
    timer->event_flag = 0;
  }
}

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
uint8_t osal_start_timer(const struct osal_tcb *task, uint16_t event_id,
                         uint16_t timeout, bool oneshot) {
  struct osal_timer *timer_new;

  // 进入临界区
  hal_reg_t cpusr = hal_enter_critical();

  // 添加定时器
  timer_new = osal_add_timer(task, event_id, timeout);

  // 创建成功，如果非单次定时器，设置reload值
  if (timer_new && !oneshot) {
    timer_new->reload = timeout;
  }

  // 退出临界区
  hal_exit_critical(cpusr);

  return ((timer_new != NULL) ? SUCCESS : NO_TIMER_AVAIL);
}

/**
 * @brief 暂停定时器
 *
 * @param task_id 任务ID
 * @param event_id 事件ID
 * @return uint8_t 成功返回OK
 */
uint8_t osal_stop_timer(const struct osal_tcb *task, uint16_t event_id) {
  struct osal_timer *foundTimer;

  // 进入临界区
  hal_reg_t cpusr = hal_enter_critical();

  // 查找定时器
  foundTimer = osal_find_timer(task, event_id);

  // 删除定时器
  if (foundTimer) {
    osal_delete_timer(foundTimer);
  }

  // 退出临界区
  hal_exit_critical(cpusr);

  return ((foundTimer != NULL) ? SUCCESS : INVALID_EVENT_ID);
}

/**
 * @brief 获取定时器的timeout
 *
 * @param task_id 任务ID
 * @param event_id 事件ID
 * @return uint16_t 超时时间
 */
uint16_t osal_timer_get_timeout(const struct osal_tcb *task,
                                uint16_t event_id) {
  uint16_t rtrn = 0;
  struct osal_timer *tmr;

  // 进入临界区
  hal_reg_t cpusr = hal_enter_critical();

  // 查找定时器
  tmr = osal_find_timer(task, event_id);

  // 获取定时器timeout
  if (tmr) {
    rtrn = tmr->timeout;
  }

  // 退出临界区
  hal_exit_critical(cpusr);

  return rtrn;
}

/**
 * @brief 当前活跃定时器数量
 *
 * @return uint8_t 活跃定时器数量
 */
uint8_t osal_timer_num_active(void) { return total_timer_cnt; }

/**
 * @brief 更新系统时间，应该在tick中断中调用
 *
 * @param ms 时间，单位ms
 */
void osal_tick(uint16_t ms) {
  struct osal_timer *timer_ptr;
  struct osal_timer *prev_timer;

  // 更新系统时间
  hal_reg_t cpusr = hal_enter_critical();
  osal_current_time += ms;
  hal_exit_critical(cpusr);

  // 更新定时器状态
  if (timer_list_head != NULL) {
    struct osal_timer *timer_ptr = timer_list_head;
    struct osal_timer *prev_timer = NULL;

    while (timer_ptr) {
      struct osal_timer *timer_to_free = NULL;

      cpusr = hal_enter_critical();

      if (timer_ptr->timeout <= ms) {
        timer_ptr->timeout = 0;
      } else {
        timer_ptr->timeout = timer_ptr->timeout - ms;
      }

      // Check for reloading
      if ((timer_ptr->timeout == 0) && (timer_ptr->event_flag != 0)) {
        // Notify the task of a timeout
        osal_set_event(timer_ptr->task, timer_ptr->event_flag);

        // Reload the timer timeout value
        timer_ptr->timeout = timer_ptr->reload;
      }

      // When timeout or delete (event_flag == 0)
      if (timer_ptr->timeout == 0 || timer_ptr->event_flag == 0) {
        // Take out of list
        if (prev_timer == NULL) {
          timer_list_head = timer_ptr->next;
        } else {
          prev_timer->next = timer_ptr->next;
        }

        // Setup to free memory
        timer_to_free = timer_ptr;

        // Next
        timer_ptr = timer_ptr->next;
      } else {
        // Get next
        prev_timer = timer_ptr;
        timer_ptr = timer_ptr->next;
      }

      hal_exit_critical(cpusr);

      if (timer_to_free) {
        osal_mem_free(timer_to_free);
      }
    }
  }
}

/**
 * @brief 获取系统时间
 *
 * @return uint32_t 系统启动之后的的毫秒数
 */
uint32_t osal_millis(void) { return (osal_current_time); }
