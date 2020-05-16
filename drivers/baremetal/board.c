/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-05-16     yaxing.chen  the first version
 */

#include <board.h>
#include <stdio.h>

int entry(void)
{
    extern int main(void);
    extern void clk_init(char *clk_source, int source_freq, int target_freq);
    extern int uart_init();

    clk_init(BSP_CLOCK_SOURCE, BSP_CLOCK_SOURCE_FREQ_MHZ, BSP_CLOCK_SYSTEM_FREQ_MHZ);
    uart_init();

    setbuf(stdout, NULL);

    main();

    return 0;
}

