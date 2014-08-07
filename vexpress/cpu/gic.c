/*
 * File      : gic.c, ARM Generic Interrupt Controller
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-07-20     Bernard      first version
 */

#include <rtthread.h>
#include <realview.h>

#include "gic.h"

#define ARM_GIC_MAX_NR      NR_GIC_PBA8
#define ARM_GIC_NR_IRQS     (IRQ_PBA8_GIC_START + 64)

struct arm_gic
{
    rt_uint32_t offset;

    rt_uint32_t dist_hw_base;
    rt_uint32_t cpu_hw_base;
};
static struct arm_gic _gic_table[ARM_GIC_MAX_NR];

int arm_gic_get_active_irq(rt_uint32_t index)
{
    int irq;

    RT_ASSERT(index < ARM_GIC_MAX_NR);

    irq = GIC_CPU_INTACK(_gic_table[index].cpu_hw_base) & 0x3ff;
    irq += _gic_table[index].offset;
    return irq;
}

void arm_gic_ack(rt_uint32_t index, int irq)
{
    rt_uint32_t mask = 1 << (irq % 32);

    RT_ASSERT(index < ARM_GIC_MAX_NR);

    irq = irq - _gic_table[index].offset;
    RT_ASSERT(irq >= 0);

    GIC_DIST_ENABLE_CLEAR(_gic_table[index].dist_hw_base, irq) = mask;
    GIC_CPU_EOI(_gic_table[index].cpu_hw_base) = irq;
    GIC_DIST_ENABLE_SET(_gic_table[index].dist_hw_base, irq) = mask;
}

void arm_gic_mask(rt_uint32_t index, int irq)
{
    rt_uint32_t mask = 1 << (irq % 32);

    RT_ASSERT(index < ARM_GIC_MAX_NR);

    irq = irq - _gic_table[index].offset;
    RT_ASSERT(irq >= 0);

    GIC_DIST_ENABLE_CLEAR(_gic_table[index].dist_hw_base, irq) = mask;
}

void arm_gic_umask(rt_uint32_t index, int irq)
{
    rt_uint32_t mask = 1 << (irq % 32);

    RT_ASSERT(index < ARM_GIC_MAX_NR);

    irq = irq - _gic_table[index].offset;
    RT_ASSERT(irq >= 0);

    GIC_DIST_ENABLE_SET(_gic_table[index].dist_hw_base, irq) = mask;
}

int arm_gic_dist_init(rt_uint32_t index, rt_uint32_t dist_base, int irq_start)
{
    unsigned int max_irq, i;
    rt_uint32_t cpumask = 1 << 0;

    RT_ASSERT(index < ARM_GIC_MAX_NR);

    cpumask |= cpumask << 8;
    cpumask |= cpumask << 16;

    _gic_table[index].dist_hw_base = dist_base;
    _gic_table[index].offset = (irq_start - 1) & ~31;

    GIC_DIST_CTRL(dist_base) = 0x0;

    /* Find out how many interrupts are supported. */
    max_irq = GIC_DIST_CTR(dist_base) & 0x1f;
    max_irq = (max_irq + 1) * 32;

    /*
     * The GIC only supports up to 1020 interrupt sources.
     * Limit this to either the architected maximum, or the
     * platform maximum.
     */
    if (max_irq > 1020) max_irq = 1020;
    if (max_irq > ARM_GIC_NR_IRQS) max_irq = ARM_GIC_NR_IRQS;

    /* Set all global interrupts to be level triggered, active low. */
    for (i = 32; i < max_irq; i += 16)
        GIC_DIST_CONFIG(dist_base, i) = 0x0;

    /* Set all global interrupts to this CPU only. */
    for (i = 32; i < max_irq; i += 4)
        GIC_DIST_TARGET(dist_base, i) = cpumask;

    /* Set priority on all interrupts. */
    for (i = 0; i < max_irq; i += 4)
        GIC_DIST_PRI(dist_base, i) = 0xa0a0a0a0;

    /* Disable all interrupts. */
    for (i = 0; i < max_irq; i += 32)
        GIC_DIST_ENABLE_CLEAR(dist_base, i) = 0xffffffff;

    GIC_DIST_CTRL(dist_base) = 0x01;

    return 0;
}

int arm_gic_cpu_init(rt_uint32_t index, rt_uint32_t cpu_base)
{
    RT_ASSERT(index < ARM_GIC_MAX_NR);

    _gic_table[index].cpu_hw_base = cpu_base;

    GIC_CPU_PRIMASK(cpu_base) = 0xf0;
    /* Enable CPU interrupt */
    GIC_CPU_CTRL(cpu_base) = 0x01;

    return 0;
}

