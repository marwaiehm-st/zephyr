.. _stm32n6570_dk_board:

STM32N6570 Discovery Kit
########################

Overview
********


Hardware
********


Supported Features
==================

The Zephyr stm32n6570_dk board configuration supports the following hardware features:

+-----------+------------+-------------------------------------+
| Interface | Controller | Driver/Component                    |
+===========+============+=====================================+
+-----------+------------+-------------------------------------+
| GPIO      | on-chip    | gpio                                |
+-----------+------------+-------------------------------------+
| UART      | on-chip    | serial port-polling;                |
|           |            | serial port-interrupt               |
+-----------+------------+-------------------------------------+


Other hardware features are not yet supported on this Zephyr port.

The default configuration can be found in the defconfig file:
:zephyr_file:`boards/st/stm32n6570_dk/stm32n6570_dk_defconfig`


Connections and IOs
===================

Programming and Debugging
*************************


Flashing
========


Flashing an application to STM32N6570-DK
------------------------------------------


Debugging
=========


Building a secure/non-secure with Arm |reg| TrustZone |reg|
===========================================================

