#include "hardware.h"

typedef void (*handler_func_t)(void *param, ...); // I don't know yet the best way to phrase the generic function type of a handler
handler_func_t InterruptVectorTable[TRAP_VECTOR_SIZE]; // the interrupt vector table is an array of interrupt handlers (type handler_t)

/**
 * @brief 
 * 
 * @param param 
 * @param ... 
 */
void trap_kernel_handler(void *param, ...); // handler for trap kernel

/**
 * @brief 
 * 
 * @param param 
 * @param ... 
 */
void trap_clock_handler(void *param, ...); // handler for trap clock

/**
 * @brief 
 * 
 * @param param 
 * @param ... 
 */
void trap_illegal_handler(void *param, ...); // handler for trap illegal

/**
 * @brief 
 * 
 * @param param 
 * @param ... 
 */
void trap_memory_handler(void *param, ...); // handler for trap memory

/**
 * @brief 
 * 
 * @param param 
 * @param ... 
 */
void trap_math_handler(void *param, ...); // handler for trap math

/**
 * @brief 
 * 
 * @param param 
 * @param ... 
 */
void trap_tty_receive_handler(void *param, ...); // handler for trap tty receive

/**
 * @brief 
 * 
 * @param param 
 * @param ... 
 */
void trap_tty_transmit_handler(void *param, ...); // handler for trap tty transmit

/**
 * @brief 
 * 
 * @param param 
 * @param ... 
 */
void trap_disk_handler(void *param, ...); // handler for trap disk
