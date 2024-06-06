/*
 * Copyright (c) 2024 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief System/hardware module for STM32N6 processor
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/cache.h>
#include <zephyr/logging/log.h>

#include <stm32_ll_bus.h>
#include <stm32_ll_pwr.h>

#include <cmsis_core.h>

#define LOG_LEVEL CONFIG_SOC_LOG_LEVEL
LOG_MODULE_REGISTER(soc);

#if defined(CONFIG_STM32N6_FSBL)
extern char _vector_start[];
void *g_pfnVectors = (void *)_vector_start;

#if defined(CONFIG_PLATFORM_SPECIFIC_INIT)
void z_arm_platform_init(void)
{
	/* __disable_irq(); Could be required TBD */

#if defined(CONFIG_STM32N6_FSBL_DEBUG)
	volatile bool stop = true;
	while (stop) {
	};
#endif

	/* This is provided by STM32Cube HAL */
	SystemInit();
}
#endif
#endif

/**
 * @brief Perform basic hardware initialization at boot.
 *
 * This needs to be run from the very beginning.
 * So the init priority has to be 0 (zero).
 *
 * @return 0
 */
static int stm32n6_init(void)
{
	/* Enable caches */
	sys_cache_instr_enable();
	sys_cache_data_enable();

	/* Update CMSIS SystemCoreClock variable (HCLK) */
	/* At reset, system core clock is set to 64 MHz from HSI */
	SystemCoreClock = 64000000;

	/* Enable PWR */
	LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_PWR);

	/* Enable IOs */
	LL_PWR_EnableVddIO2();
	LL_PWR_EnableVddIO3();
	LL_PWR_EnableVddIO4();
	LL_PWR_EnableVddIO5();

	LL_PWR_SetVddIO2VoltageRange(LL_PWR_VDDIO_VOLTAGE_RANGE_1V8);
	LL_PWR_SetVddIO3VoltageRange(LL_PWR_VDDIO_VOLTAGE_RANGE_1V8);

	return 0;
}

SYS_INIT(stm32n6_init, PRE_KERNEL_1, 0);
