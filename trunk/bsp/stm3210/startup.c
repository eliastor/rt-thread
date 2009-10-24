/*
 * File      : startup.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-08-31     Bernard      first implementation
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f10x.h"
#include "board.h"
#include "rtc.h"

/**
 * @addtogroup STM32
 */

/*@{*/

#ifdef RT_USING_LWIP
#ifdef STM32F10X_CL
	extern void rt_hw_stm32_eth_init(void);
#else
	#include "enc28j60.h"
#endif
#include <netif/ethernetif.h>
#endif

extern int  rt_application_init(void);
#ifdef RT_USING_FINSH
extern void finsh_system_init(void);
extern void finsh_set_device(const char* device);
#endif

/* bss end definitions for heap init */
#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#elif __ICCARM__
#pragma section="HEAP"
#else
extern int __bss_end;
#endif

#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{
	rt_kprintf("\n\r Wrong parameter value detected on\r\n");
	rt_kprintf("       file  %s\r\n", file);
	rt_kprintf("       line  %d\r\n", line);

	while (1) ;
}
#endif

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
	/* init board */
	rt_hw_board_init();

	/* show version */
	rt_show_version();

	/* init tick */
	rt_system_tick_init();

	/* init kernel object */
	rt_system_object_init();

	/* init timer system */
	rt_system_timer_init();

#ifdef RT_USING_HEAP
#if STM32_EXT_SRAM
	rt_system_heap_init((void*)STM32_EXT_SRAM_BEGIN, (void*)STM32_EXT_SRAM_END);
#else
	#ifdef __CC_ARM
		rt_system_heap_init((void*)&Image$$RW_IRAM1$$ZI$$Limit, (void*)STM32_SRAM_END);
	#elif __ICCARM__
	    rt_system_heap_init(__segment_end("HEAP"), (void*)STM32_SRAM_END);
	#else
		/* init memory system */
		rt_system_heap_init((void*)&__bss_end, (void*)STM32_SRAM_END);
	#endif
#endif
#endif

	/* init scheduler system */
	rt_system_scheduler_init();

	/* init hardware serial device */
	rt_hw_usart_init();

#ifdef RT_USING_DFS
	/* init sdcard driver */
#if STM32_USE_SDIO
	rt_hw_sdcard_init();
#else
	rt_hw_msd_init();
#endif
#endif

#ifdef RT_USING_LWIP
	eth_system_device_init();

	/* register ethernetif device */
#ifdef STM32F10X_CL
	rt_hw_stm32_eth_init();
#else
	rt_hw_enc28j60_init();
#endif
#endif

    rt_hw_rtc_init();

	/* init all device */
	rt_device_init_all();

	/* init application */
	rt_application_init();

#ifdef RT_USING_FINSH
	/* init finsh */
	finsh_system_init();
	finsh_set_device(FINSH_DEVICE_NAME);
#endif

	/* init idle thread */
	rt_thread_idle_init();

	/* start scheduler */
	rt_system_scheduler_start();

	/* never reach here */
	return ;
}

int main(void)
{
	rt_uint32_t UNUSED level;

	/* disable interrupt first */
	level = rt_hw_interrupt_disable();

	/* init system setting */
	SystemInit();

	/* startup RT-Thread RTOS */
	rtthread_startup();

	return 0;
}

/*@}*/
