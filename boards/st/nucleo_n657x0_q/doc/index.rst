.. _nucleo_n657x0_q_board:

ST Nucleo N657X0-Q
##################

Overview
********

The default configuration per core can be found in the defconfig files:
:zephyr_file:`boards/st/nucleo_n657x0_q/nucleo_n657x0_q_defconfig`
:zephyr_file:`boards/st/nucleo_n657x0_q/nucleo_n657x0_q_fsbl_defconfig`

Default Zephyr Peripheral Mapping:
----------------------------------

See device tree.

System Clock
------------

ST Nucleo N657X0-Q System Clock is running at 400MHz,
clocked by a PLL driven by 64MHz high speed internal oscillator.

Serial Port
-----------

The Zephyr console output is assigned to UART1. Default settings are 115200 8N1.

Chip version
------------

If the chip in use is one of 1.0 or 1.1, make sure kconfig:option:`CONFIG_STM32N6_CUT1`.

Programming and Debugging
*************************

ST Nucleo N657X0-Q supports two modes of operation. Each mode being made available
as a specific board variant.

  - A developer mode where the binary is running directly from RAM. In this mode,
  Zephyr application binary is running in fully non secure mode. This mode is available
  through the default nucleo_n657x0_q variant
  - A First Stage BootLoader mode (FSBL), in which Zephyr application binary is
  initially copied from external flash to RAM by the BootROM before being run in
  RAM in fully secure mode. This mode is available through the invocation of
  nucleo_n657x0_q//fsbl variant

Depending on the mode in use, flashing and debug operations will differ.

Building
========

In both modes, building can be done using the traditional `west flash` command.
Here is an example for the :zephyr:code-sample:`blinky` application:

In developper mode:

.. zephyr-app-commands::
   :zephyr-app: samples/basic/blinky
   :board: nucleo_n657x0_q
   :goals: build

In FSBL mode:

.. zephyr-app-commands::
   :zephyr-app: samples/basic/blinky
   :board: nucleo_n657x0_q//fsbl
   :goals: build

.. note::
   In that mode, binary signing operation will be done automatically following the
   build operation. This signature is requested by the BootROM.

.. note::
   Signing operation requires a STM32N6 compatible STM32CubeProgrammer version to
   be installed and available in 'path' global variable.

Flashing and debugging in developper mode
=========================================

In this mode, flashing and debugging is only available using the compatible version
of STM32CubeIDE tool. For that:

- Create an empty stm32n6 project by going to File > New > STM32 project

- Select your board, click Next, select 'Boot' Project Structure and Empty project.

- Right click on your project name, select Debug as > Debug configurations

- In the new window, create a new target in STM32 Cortex-M C/C++ Application

- Select the new target and enter the path to zephyr.elf file in the C/C++ Application field

- Check "Disable auto build"

- Run debug

Flashing in FSBL mode
=====================

In this mode, flashing can be done using usual `west flash` command.
Here is an example for the :zephyr:code-sample:`blinky` application.

.. zephyr-app-commands::
   :zephyr-app: samples/basic/blinky
   :board: nucleo_n657x0_q//fsbl
   :goals: flash

.. note::
   Make sure to set the Boot0/1 jumpers in dev mode before flashing and move them
   back in FSBL mode after flash for execution.

.. note::
   Flashing operation requires a STM32N6 compatible STM32CubeProgrammer version to
   be installed and available in 'path' global variable.

Debugging in FSBL mode
======================

In this mode, you can also debug an application using a STM32N6 compatible version
of STM32CubeIDE. Though it requires a specific tweak:

- Enable kconfig:option:`CONFIG_STM32N6_FSBL_DEBUG`. This option will make the software
block in a while loop at boot. Make sure to disable this option when not debugging.

Then, To debug using STM32CubeIDE:

- Create an empty stm32n6 project by going to File > New > STM32 project

- Select your board, click Next, select 'Boot' Project Structure and Empty project.

- Right click on your project name, select Debug as > Debug configurations

- In the new window, create a new target in STM32 Cortex-M C/C++ Application

- Select the new target and enter the path to zephyr.elf file in the C/C++ Application field

- Check "Disable auto build"

- In the "Startup" tab, edit the Load Image and unselect "Download" option.

- Save

- Run debug

- Set the `stop` volatile variable from the kconfig:option:`CONFIG_STM32N6_FSBL_DEBUG`
  while loop to `false` to continue the execution.
