# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2024 STMicroelectronics

if(CONFIG_STM32N6_FSBL)

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    set(SIGNING_TOOL STM32MP_SigningTool_CLI.exe)
else()
    set(SIGNING_TOOL STM32MP_SigningTool_CLI)
endif()

set_property(GLOBAL APPEND PROPERTY extra_post_build_commands
  COMMAND ${SIGNING_TOOL}
  -bin ${PROJECT_BINARY_DIR}/${CONFIG_KERNEL_BIN_NAME}.bin
  -nk -of 0x80000000 -t fsbl -hv 2.1
  -o ${PROJECT_BINARY_DIR}/${CONFIG_KERNEL_BIN_NAME}_signed.bin
  -dump ${PROJECT_BINARY_DIR}/${CONFIG_KERNEL_BIN_NAME}_signed.bin
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)

set_property(TARGET runners_yaml_props_target PROPERTY bin_file zephyr_signed.bin)

board_runner_args(stm32cubeprogrammer "--port=swd")
board_runner_args(stm32cubeprogrammer "--tool-opt= mode=HOTPLUG ap=1")
board_runner_args(stm32cubeprogrammer "--extload=MX66UW1G45G_STM32N6570-DK.stldr")

include(${ZEPHYR_BASE}/boards/common/stm32cubeprogrammer.board.cmake)
endif()
