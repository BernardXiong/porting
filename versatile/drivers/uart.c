/*
 *  UART driver
 *
 * COPYRIGHT (C) 2013, Shanghai Real-Thread Technology Co., Ltd
 *
 *  This file is part of RT-Thread (http://www.rt-thread.org)
 *  Maintainer: bernard.xiong <bernard.xiong at gmail.com>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-03-30     Bernard      the first verion
 */

#include <rthw.h>
#include <rtdevice.h>

#include "uart.h"

struct hw_uart_device
{
	rt_uint32_t hw_base;
	rt_uint32_t irqno;
};

#define UART_DR(base)	__REG32(base + 0x00)
#define UART_FR(base)	__REG32(base + 0x18)
#define UART_CR(base)	__REG32(base + 0x30)
#define UART_IMSC(base)	__REG32(base + 0x38)
#define UART_ICR(base)	__REG32(base + 0x44)

#define UARTFR_RXFE   	0x10
#define UARTFR_TXFF   	0x20
#define UARTIMSC_RXIM 	0x10
#define UARTIMSC_TXIM 	0x20
#define UARTICR_RXIC  	0x10
#define UARTICR_TXIC  	0x20

static void rt_hw_uart_isr(int irqno, void* param)
{
    struct rt_serial_device* serial = (struct rt_serial_device*)param;

    rt_hw_serial_isr(serial);
}

static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
	return RT_EOK;
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct hw_uart_device* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct hw_uart_device *)serial->parent.user_data;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        UART_IMSC(uart->hw_base) &= ~UARTIMSC_RXIM;
        break;

    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        UART_IMSC(uart->hw_base) |= UARTIMSC_RXIM;
        rt_hw_interrupt_install(uart->irqno, rt_hw_uart_isr, serial, "uart");
        rt_hw_interrupt_umask(uart->irqno);
        break;
    }

    return RT_EOK;
}

static int uart_putc(struct rt_serial_device *serial, char c)
{
    struct hw_uart_device* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct hw_uart_device *)serial->parent.user_data;

    while (UART_FR(uart->hw_base) & UARTFR_TXFF);
    UART_DR(uart->hw_base) = c;

    return 1;
}

static int uart_getc(struct rt_serial_device *serial)
{
    int ch;
    struct hw_uart_device* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct hw_uart_device *)serial->parent.user_data;

    ch = -1;
    if (!(UART_FR(uart->hw_base) & UARTFR_RXFE))
    {
        ch = UART_DR(uart->hw_base) & 0xff;
    }

    return ch;
}

static const struct rt_uart_ops _uart_ops =
{
    uart_configure,
    uart_control,
    uart_putc,
    uart_getc,
};

/* UART device driver structure */
static struct serial_ringbuffer _uart_int_rx;
static struct hw_uart_device _uart_device =
{
    VERSATILE_UART0_BASE,
    INT_UARTINT0,
};
static struct rt_serial_device _serial;

void rt_hw_uart_init(void)
{
    struct hw_uart_device* uart;
    struct serial_configure config;

    uart = &_uart_device;
    config.baud_rate = BAUD_RATE_115200;
    config.bit_order = BIT_ORDER_LSB;
    config.data_bits = DATA_BITS_8;
    config.parity    = PARITY_NONE;
    config.stop_bits = STOP_BITS_1;
    config.invert    = NRZ_NORMAL;

    _serial.ops    = &_uart_ops;
    _serial.int_rx = &_uart_int_rx;
    _serial.config = config;

    /* register UART1 device */
    rt_hw_serial_register(&_serial, "uart",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
                          uart);
    /* enable Rx and Tx of UART */
    UART_CR(uart->hw_base) = (1 << 0) | (1 << 8) | (1 << 9);
}
