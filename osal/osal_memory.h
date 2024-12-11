/**
 * @file osal_memory.h
 * @author ljgabc
 * @brief 动态内存管理器
 * @version 0.1
 * @date 2024-11-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "osal_types.h"
#include "osal_config.h"

// 使能内存使用情况统计功能
#ifndef OSALMEM_METRICS
#define OSALMEM_METRICS 0
#endif

// 使能内存申请与释放跟踪打印功能
#ifndef DPRINTF_OSALHEAPTRACE
#define DPRINTF_OSALHEAPTRACE 0
#endif

// 使能内存性能统计功能
#ifndef OSALMEM_PROFILER
#define OSALMEM_PROFILER 0
#endif

// 使能常驻内存性能统计功能
#ifndef OSALMEM_PROFILER_LL
#define OSALMEM_PROFILER_LL 0
#endif

/*
 * 初始化内存管理器
 */
void osal_mem_init(void);

/*
 * 当常驻内存申请完成后，调节空闲内存指针位置，提高内存申请效率
 * 应用程序需要在系统任务创建和初始化完成后调用此函数
 */
void osal_mem_kick(void);


/**
 * @brief 申请内存
 * 
 * @param size 期望申请的内存大小Byte
 * @return void* 成功返回申请到的内存地址，失败返回NULL
 */
#if DPRINTF_OSALHEAPTRACE
void *osal_mem_alloc_dbg(uint16_t size, const char *fname, unsigned lnum);
#define osal_mem_alloc(_size) osal_mem_alloc_dbg(_size, __FILE__, __LINE__)
#else
void *osal_mem_alloc(uint16_t size);
#endif

/**
 * @brief 释放内存
 * 
 * @param ptr 通过osal_mem_alloc申请到的内存地址
 */
#if DPRINTF_OSALHEAPTRACE
void osal_mem_free_dbg(void *ptr, const char *fname, unsigned lnum);
#define osal_mem_free(_ptr) osal_mem_free_dbg(_ptr, __FILE__, __LINE__)
#else
void osal_mem_free(void *ptr);
#endif

#if (OSALMEM_METRICS)
/*
 * Return the maximum number of blocks ever allocated at once.
 */
uint16_t osal_heap_block_max(void);

/*
 * Return the current number of blocks now allocated.
 */
uint16_t osal_heap_block_cnt(void);

/*
 * Return the current number of free blocks.
 */
uint16_t osal_heap_block_free(void);

/*
 * Return the current number of bytes allocated.
 */
uint16_t osal_heap_mem_used(void);

/*
 * Return the highest number of bytes ever used in the heap.
 */
uint16_t osal_heap_high_water(void);
#endif

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

