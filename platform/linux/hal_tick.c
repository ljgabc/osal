/**
 * @file hal_tick.c
 * @author ljgabc
 * @brief Linux平台下tick实现，在一个线程中定时调用
 * @version 0.1
 * @date 2024-11-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hal_tick.h"
#include "osal_timer.h"

static pthread_t hal_timer_pthread_fd;

/**
 * 定时器线程，为osal提供滴答心跳
 */
static void *hal_timer_pthread(void *pro) {
  pro = pro;
  while (1) {
    usleep(HAL_TICK_PERIOD_MS * 1000UL);
    osal_update_timers();
  }
  return 0;
}

/**
 * @brief 定时器初始化，设定系统时钟
 */
void hal_tick_init(void) {
  // 创建定时器线程，使用线程来模拟定时器
  int ret =
      pthread_create(&hal_timer_pthread_fd, NULL, hal_timer_pthread, NULL);
  if (ret != 0) {
    perror("Create hal timer error");
    exit(1);
  }
  printf("Init hal timer ok !\n");
}

/**
 * @brief
 * 开启tick，OSAL会根据程序中软件定时器的实际使用动态开启和关闭，为空则一直开启
 */
void hal_tick_start(void) {}

/**
 * @brief 关闭tick，为空则一直不关闭
 */
void hal_tick_stop(void) {}
