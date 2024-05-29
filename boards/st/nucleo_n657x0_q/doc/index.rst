.. _nucleo_n657x0_q_board:

ST Nucleo N657X0-Q
###################

Overview
********

Hardware
********

Nucleo N657X0-Q provides the following hardware components:

Supported Features
==================

The Zephyr nucleo_n657x0_q board configuration supports the following hardware
features:

+-------------+------------+-------------------------------------+
| Interface   | Controller | Driver/Component                    |
+=============+============+=====================================+
| NVIC        | on-chip    | nested vector interrupt controller  |
+-------------+------------+-------------------------------------+
| UART        | on-chip    | serial port                         |
+-------------+------------+-------------------------------------+
| PINMUX      | on-chip    | pinmux                              |
+-------------+------------+-------------------------------------+
| GPIO        | on-chip    | gpio                                |
+-------------+------------+-------------------------------------+

Other hardware features are not yet supported on this Zephyr port.

The default configuration per core can be found in the defconfig files:
:zephyr_file:`boards/st/nucleo_n657x0_q/nucleo_n657x0_q_defconfig`


Default Zephyr Peripheral Mapping:
----------------------------------

System Clock
------------

Serial Port
-----------

The Zephyr console output is assigned to UART1. Default settings are 115200 8N1.


Programming and Debugging
*************************

Flashing
========

Debugging
=========
