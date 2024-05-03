/*
 * Copyright (c) 2024 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT st_stm32_xspi_psram

#include <zephyr/device.h>
#include <soc.h>
#include <errno.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/drivers/clock_control/stm32_clock_control.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "memc_stm32_xspi_psram.h"

LOG_MODULE_REGISTER(memc_stm32_xspi_psram, CONFIG_MEMC_LOG_LEVEL);

#define STM32_XSPI_NODE DT_INST_PARENT(0)

#define BUFFERSIZE	10240

struct memc_stm32_xspi_psram_config {
	const struct pinctrl_dev_config *pcfg;
	const struct stm32_pclken pclken; /* clock subsystem */
	const struct stm32_pclken pclken_ker; /* clock subsystem */
	const struct stm32_pclken pclken_mgr; /* clock subsystem */
	size_t memory_size;
};

struct memc_stm32_xspi_psram_data {
	XSPI_HandleTypeDef hxspi;
};

uint32_t ap_write_reg(XSPI_HandleTypeDef *hxspi, uint32_t address, uint8_t *value)
{
	XSPI_RegularCmdTypeDef command = {0};

	/* Initialize the write register command */
	command.OperationType		= HAL_XSPI_OPTYPE_COMMON_CFG;
	command.Instruction		= WRITE_REG_CMD;
	command.InstructionMode		= HAL_XSPI_INSTRUCTION_8_LINES;
	command.InstructionWidth	= HAL_XSPI_INSTRUCTION_8_BITS;
	command.InstructionDTRMode	= HAL_XSPI_INSTRUCTION_DTR_DISABLE;
	command.Address			= address;
	command.AddressMode		= HAL_XSPI_ADDRESS_8_LINES;
	command.AddressWidth		= HAL_XSPI_ADDRESS_32_BITS;
	command.AddressDTRMode		= HAL_XSPI_ADDRESS_DTR_ENABLE;
	command.AlternateBytesMode	= HAL_XSPI_ALT_BYTES_NONE;
	command.DataMode		= HAL_XSPI_DATA_8_LINES;
	command.DataLength		= 2;
	command.DataDTRMode		= HAL_XSPI_DATA_DTR_ENABLE;
	command.DummyCycles		= 0;
	command.DQSMode			= HAL_XSPI_DQS_DISABLE;

	/* Configure the command */
	if (HAL_XSPI_Command(hxspi, &command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return -EIO;
	}

	/* Transmission of the data */
	if (HAL_XSPI_Transmit(hxspi, (uint8_t *)value, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return -EIO;
	}

	return 0;
}

uint32_t ap_read_reg(XSPI_HandleTypeDef *hxspi, uint32_t address, uint8_t *value,
			 uint32_t latency_cycles)
{
	XSPI_RegularCmdTypeDef command;

	/* Initialize the read register command */
	command.OperationType		= HAL_XSPI_OPTYPE_COMMON_CFG;
	command.Instruction		= READ_REG_CMD;
	command.InstructionMode		= HAL_XSPI_INSTRUCTION_8_LINES;
	command.InstructionWidth	= HAL_XSPI_INSTRUCTION_8_BITS;
	command.InstructionDTRMode	= HAL_XSPI_INSTRUCTION_DTR_DISABLE;
	command.Address			= address;
	command.AddressMode		= HAL_XSPI_ADDRESS_8_LINES;
	command.AddressWidth		= HAL_XSPI_ADDRESS_32_BITS;
	command.AddressDTRMode		= HAL_XSPI_ADDRESS_DTR_ENABLE;
	command.AlternateBytesMode	= HAL_XSPI_ALT_BYTES_NONE;
	command.DataMode		= HAL_XSPI_DATA_8_LINES;
	command.DataLength		= 2;
	command.DataDTRMode		= HAL_XSPI_DATA_DTR_ENABLE;
	command.DummyCycles		= latency_cycles;
	command.DQSMode			= HAL_XSPI_DQS_ENABLE;

	/* Configure the command */
	if (HAL_XSPI_Command(hxspi, &command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return -EIO;
	}

	/* Reception of the data */
	if (HAL_XSPI_Receive(hxspi, (uint8_t *)value, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return -EIO;
	}

	return 0;
}

static int configure_ap_memory(XSPI_HandleTypeDef *hxspi)
{
	uint8_t read_latency_code = DT_INST_PROP(0, read_latency);
	uint8_t read_latency_cycles = read_latency_code + 3U; // Code 0 <=> 3 cycles...

	/* MR0 register for read and write */
	uint8_t regW_MR0[2] = {(DT_INST_PROP(0, fixed_latency) ? 0x20 : 0x00) |
			       (read_latency_code << 2) |
			       (DT_INST_PROP(0, drive_strength)), 0x8D};
	uint8_t regR_MR0[2] = {0};

	/* MR4 register for read and write */
	uint8_t regW_MR4[2] = {(DT_INST_PROP(0, write_latency) << 5) |
			      (DT_INST_PROP(0, refresh_rate) << 3) |
			      (DT_INST_PROP(0, pasr)), 0x05};
	uint8_t regR_MR4[2] = {0};

	/* MR8 register for read and write */
	uint8_t regW_MR8[2] = {(DT_INST_PROP(0, io_x16_mode) ? 0x40 : 0x00) |
			       (DT_INST_PROP(0, rbx) ? 0x08 : 0x00) |
			       (DT_INST_PROP(0, burst_type_hybrid_wrap) ? 0x04 : 0x00) |
			       (DT_INST_PROP(0, burst_length)), 0x08};
	uint8_t regR_MR8[2] = {0};

	/* Configure Read Latency and drive Strength */
	if (ap_write_reg(hxspi, MR0, regW_MR0) != HAL_OK) {
		return -EIO;
	}

	/* Check MR0 configuration */
	if (ap_read_reg(hxspi, MR0, regR_MR0, read_latency_cycles) != HAL_OK) {
		return -EIO;
	}
	if (regR_MR0[0] != regW_MR0[0]) {
		return -EIO;
	}

	/* Configure Write Latency and refresh rate */
	if (ap_write_reg(hxspi, MR4, regW_MR4) != HAL_OK) {
		return -EIO;
	}

	/* Check MR4 configuration */
	if (ap_read_reg(hxspi, MR4, regR_MR4, read_latency_cycles) != HAL_OK) {
		return -EIO;
	}
	if (regR_MR4[0] != regW_MR4[0]) {
		return -EIO;
	}

	/* Configure Burst Length */
	if (ap_write_reg(hxspi, MR8, regW_MR8) != HAL_OK) {
		return -EIO;
	}

	/* Check MR8 configuration */
	if (ap_read_reg(hxspi, MR8, regR_MR8, read_latency_cycles) != HAL_OK) {
		return -EIO;
	}
	if (regR_MR8[0] != regW_MR8[0]) {
		return -EIO;
	}

	return 0;

}

static int memc_stm32_xspi_psram_init(const struct device *dev)
{
	const struct memc_stm32_xspi_psram_config *dev_cfg = dev->config;
	struct memc_stm32_xspi_psram_data *dev_data = dev->data;
	uint32_t ahb_clock_freq;
	XSPIM_CfgTypeDef cfg;
	XSPI_RegularCmdTypeDef command;
	XSPI_MemoryMappedTypeDef mem_mapped_cfg;
	int ret;

	/* Signals configuration */
	ret = pinctrl_apply_state(dev_cfg->pcfg, PINCTRL_STATE_DEFAULT);
	if (ret < 0) {
		LOG_ERR("XSPI pinctrl setup failed (%d)", ret);
		return ret;
	}

	if (!device_is_ready(DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE))) {
		LOG_ERR("clock control device not ready");
		return -ENODEV;
	}

	/* Clock configuration */
	if (clock_control_on(DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE),
			     (clock_control_subsys_t) &dev_cfg->pclken) != 0) {
		LOG_ERR("Could not enable XSPI clock");
		return -EIO;
	}

	/* Alternate clock config for peripheral if any */
#if DT_CLOCKS_HAS_NAME(STM32_XSPI_NODE, xspi_ker)
	if (clock_control_configure(DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE),
				    (clock_control_subsys_t) &dev_cfg->pclken_ker,
				    NULL) != 0) {
		LOG_ERR("Could not select XSPI domain clock");
		return -EIO;
	}
	if (clock_control_get_rate(DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE),
				   (clock_control_subsys_t) &dev_cfg->pclken_ker,
				   &ahb_clock_freq) < 0) {
		LOG_ERR("Failed call clock_control_get_rate(pclken_ker)");
		return -EIO;
	}
#else
	if (clock_control_get_rate(DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE),
				   (clock_control_subsys_t) &dev_cfg->pclken,
				   &ahb_clock_freq) < 0) {
		LOG_ERR("Failed call clock_control_get_rate(pclken)");
		return -EIO;
	}
#endif
#if DT_CLOCKS_HAS_NAME(STM32_XSPI_NODE, xspi_mgr)
	if (clock_control_on(DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE),
			     (clock_control_subsys_t) &dev_cfg->pclken_mgr) != 0) {
		LOG_ERR("Could not enable XSPI Manager clock");
		return -EIO;
	}
#endif

	dev_data->hxspi.Init.MemorySize = find_msb_set(dev_cfg->memory_size) - 2;

	if (HAL_XSPI_Init(&dev_data->hxspi) != HAL_OK) {
		LOG_ERR("XSPI Init failed");
		return -EIO;
	}

	cfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
	cfg.IOPort = HAL_XSPIM_IOPORT_1;

	if (HAL_XSPIM_Config(&dev_data->hxspi, &cfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK){
		return -EIO;
	}

	/* Configure AP memory registers */
	ret = configure_ap_memory(&dev_data->hxspi);
	if (ret != HAL_OK) {
		LOG_ERR("AP memory configuration failed");
		return -EIO;
	}

	command.OperationType		= HAL_XSPI_OPTYPE_WRITE_CFG;
	command.InstructionMode		= HAL_XSPI_INSTRUCTION_8_LINES;
	command.InstructionWidth	= HAL_XSPI_INSTRUCTION_8_BITS;
	command.InstructionDTRMode	= HAL_XSPI_INSTRUCTION_DTR_DISABLE;
	command.Instruction		= WRITE_CMD;
	command.AddressMode		= HAL_XSPI_ADDRESS_8_LINES;
	command.AddressWidth		= HAL_XSPI_ADDRESS_32_BITS;
	command.AddressDTRMode		= HAL_XSPI_ADDRESS_DTR_ENABLE;
	command.Address			= 0x0;
	command.AlternateBytesMode	= HAL_XSPI_ALT_BYTES_NONE;
	command.DataMode		= HAL_XSPI_DATA_16_LINES;
	command.DataDTRMode		= HAL_XSPI_DATA_DTR_ENABLE;
	command.DataLength		= BUFFERSIZE;
	command.DummyCycles		= DUMMY_CLOCK_CYCLES_WRITE;
	command.DQSMode			= HAL_XSPI_DQS_ENABLE;

	if (HAL_XSPI_Command(&dev_data->hxspi, &command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return -EIO;
	}

	command.OperationType	= HAL_XSPI_OPTYPE_READ_CFG;
	command.Instruction	= READ_CMD;
	command.DummyCycles	= DUMMY_CLOCK_CYCLES_READ;
	command.DQSMode		= HAL_XSPI_DQS_ENABLE;

	if (HAL_XSPI_Command(&dev_data->hxspi, &command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return -EIO;
	}

	mem_mapped_cfg.TimeOutActivation	= HAL_XSPI_TIMEOUT_COUNTER_ENABLE;
	mem_mapped_cfg.TimeoutPeriodClock	= 0x34;


	if (HAL_XSPI_MemoryMapped(&dev_data->hxspi, &mem_mapped_cfg) != HAL_OK) {
		return -EIO;
	}

	return 0;
}

PINCTRL_DT_DEFINE(STM32_XSPI_NODE);

static const struct memc_stm32_xspi_psram_config memc_stm32_xspi_cfg = {
	.pcfg = PINCTRL_DT_DEV_CONFIG_GET(STM32_XSPI_NODE),
	.pclken = {.bus = DT_CLOCKS_CELL_BY_NAME(STM32_XSPI_NODE, xspix, bus),
		   .enr = DT_CLOCKS_CELL_BY_NAME(STM32_XSPI_NODE, xspix, bits)},
#if DT_CLOCKS_HAS_NAME(STM32_XSPI_NODE, xspi_ker)
	.pclken_ker = {.bus = DT_CLOCKS_CELL_BY_NAME(STM32_XSPI_NODE, xspi_ker, bus),
		       .enr = DT_CLOCKS_CELL_BY_NAME(STM32_XSPI_NODE, xspi_ker, bits)},
#endif
#if DT_CLOCKS_HAS_NAME(STM32_XSPI_NODE, xspi_mgr)
	.pclken_mgr = {.bus = DT_CLOCKS_CELL_BY_NAME(STM32_XSPI_NODE, xspi_mgr, bus),
		       .enr = DT_CLOCKS_CELL_BY_NAME(STM32_XSPI_NODE, xspi_mgr, bits)},
#endif
	.memory_size = DT_INST_REG_ADDR_BY_IDX(0, 1),
};

static struct memc_stm32_xspi_psram_data memc_stm32_xspi_data = {
	.hxspi = {
		.Instance = (XSPI_TypeDef *)DT_REG_ADDR(STM32_XSPI_NODE),
		.Init = {
			.FifoThresholdByte = 4,
			.MemoryMode = HAL_XSPI_SINGLE_MEM,
			.MemoryType = (DT_INST_PROP(0, io_x16_mode) ?
					HAL_XSPI_MEMTYPE_APMEM_16BITS :
					HAL_XSPI_MEMTYPE_APMEM),
			.ChipSelectHighTimeCycle = 1,
			.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE,
			.ClockMode = HAL_XSPI_CLOCK_MODE_0,
			.WrapSize = HAL_XSPI_WRAP_NOT_SUPPORTED,
			.ClockPrescaler = 1,
			.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE,
			.DelayHoldQuarterCycle = HAL_XSPI_DHQC_DISABLE,
			.ChipSelectBoundary = HAL_XSPI_BONDARYOF_NONE,
			.MaxTran = 0,
			.Refresh = 0,
			.MemorySelect = HAL_XSPI_CSSEL_NCS1,
		},
	},
};

DEVICE_DT_INST_DEFINE(0, &memc_stm32_xspi_psram_init, NULL,
		      &memc_stm32_xspi_data, &memc_stm32_xspi_cfg,
		      POST_KERNEL, CONFIG_MEMC_INIT_PRIORITY,
		      NULL);
