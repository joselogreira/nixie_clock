---
title: Voltages and power
permalink: /docs/power/
---

__[PDF schematic](/nixie_clock/assets/pdf/nixie_clock_3.pdf "Schematic")__

The 160V high voltage source for the Nixie tubes is a [boost converter](https://en.wikipedia.org/wiki/Boost_converter) based around the __[TPS40210](https://www.ti.com/product/TPS40210)__ from Texas Instruments. It handles all the boost control by itself, including overcurrent. The MCU enables or disables the converter operation.

![Boost converter](/nixie_clock/assets/img/boost.png "Boost Converter")

The boost converter operates in Continuous Conduction Mode (CCM), its Duty Cycle is close to _0.95_, and switching frequency is around _100KHz_. It may seem an exreme operation case (due to its high voltage output), but keep in mind that individual digits glow with a current less than _2mA_ with full brightness. Besides, due to the [multiplexing](/nixie_clock/docs/multiplexing/) scheme, only one tube is active at a time. Thus, even when the voltage requirement is very high, the power requirement is very low.

The digital circuitry is powered with a simple linear regulator, but not directly from the regulator output: when external power is not present, the coin cell backup battery powers the MCU. This switching between these two sources is automatically performed using shottky diodes `D11` and `D12`. The presence of external power is sensed through `R30` and `R31`. That way, the MCU switches to [sleep mode](/nixie_clock/docs/sleep/) when no external power is sensed.

![Linear regulator](/nixie_clock/assets/img/ldo_reg.png "Linear Regulator and Battery")