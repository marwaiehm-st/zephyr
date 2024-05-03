/*
 * Copyright (c) 2024 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MEMC_STM32_XSPI_PSRAM_H__
#define __MEMC_STM32_XSPI_PSRAM_H__

/* Registers definition */
#define MR0	0x00000000U
#define MR1	0x00000001U
#define MR2	0x00000002U
#define MR3	0x00000003U
#define MR4	0x00000004U
#define MR8	0x00000008U

#define READ_CMD	0x00
#define WRITE_CMD	0x80

#define READ_REG_CMD	0x40
#define WRITE_REG_CMD	0xC0

/* Default dummy clocks cycles */
#define DUMMY_CLOCK_CYCLES_READ		4
#define DUMMY_CLOCK_CYCLES_WRITE	4

#endif /* __MEMC_STM32_XSPI_PSRAM_H__ */