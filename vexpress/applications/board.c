/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-11-20     Bernard    the first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <components.h>

#include "board.h"

#define TIMER_LOAD(hw_base)              __REG32(hw_base + 0x00)
#define TIMER_VALUE(hw_base)             __REG32(hw_base + 0x04)
#define TIMER_CTRL(hw_base)              __REG32(hw_base + 0x08)
#define TIMER_CTRL_ONESHOT      (1 << 0)
#define TIMER_CTRL_32BIT        (1 << 1)
#define TIMER_CTRL_DIV1         (0 << 2)
#define TIMER_CTRL_DIV16        (1 << 2)
#define TIMER_CTRL_DIV256       (2 << 2)
#define TIMER_CTRL_IE           (1 << 5)        /* Interrupt Enable (versatile only) */
#define TIMER_CTRL_PERIODIC     (1 << 6)
#define TIMER_CTRL_ENABLE       (1 << 7)

#define TIMER_INTCLR(hw_base)            __REG32(hw_base + 0x0c)
#define TIMER_RIS(hw_base)               __REG32(hw_base + 0x10)
#define TIMER_MIS(hw_base)               __REG32(hw_base + 0x14)
#define TIMER_BGLOAD(hw_base)            __REG32(hw_base + 0x18)

#define SYS_CTRL                        __REG32(REALVIEW_SCTL_BASE)

static void rt_hw_timer_isr(int vector, void *param)
{
    rt_tick_increase();

    /* clear interrupt */
    TIMER_INTCLR(REALVIEW_TIMER0_1_BASE) = 0x01;
}

int rt_hw_timer_init(void)
{
    rt_uint32_t val;

    /*
     * set clock frequency:
     *      REALVIEW_REFCLK is 32KHz
     *      REALVIEW_TIMCLK is 1MHz
     */
    // SYS_CTRL |= REALVIEW_REFCLK;
    TIMER_LOAD(REALVIEW_TIMER0_1_BASE) = 1000;

    /* disable Timer0 */
    val = TIMER_CTRL(REALVIEW_TIMER0_1_BASE);
    val &= ~TIMER_CTRL_ENABLE;
    TIMER_CTRL(REALVIEW_TIMER0_1_BASE) = val;

    /* Setup Timer0 for generating irq */
    val = TIMER_CTRL(REALVIEW_TIMER0_1_BASE);
    val &= ~TIMER_CTRL_ENABLE;
    val |= (TIMER_CTRL_ENABLE | TIMER_CTRL_32BIT | TIMER_CTRL_PERIODIC | TIMER_CTRL_IE);
    TIMER_CTRL(REALVIEW_TIMER0_1_BASE) = val;

    rt_hw_interrupt_install(IRQ_PBA8_TIMER0_1, rt_hw_timer_isr, RT_NULL, "tick");
    rt_hw_interrupt_umask(IRQ_PBA8_TIMER0_1);

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_timer_init);

/**
 * This function will initialize beaglebone board
 */
void rt_hw_board_init()
{
    // rt_components_board_init();
    // rt_hw_timer_init();
    rt_hw_uart_init();

    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
}

/*@}*/

