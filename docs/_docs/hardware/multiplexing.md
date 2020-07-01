---
title: Tubes' discrete multiplexing
permalink: /docs/multiplexing/
---

__[PDF schematic](/nixie_clock/assets/pdf/nixie_clock_3.pdf "Schematic")__

Multiplexing is mandatory. There's just not enough pins in the MCU to directly drive evey tube.

To multiplex tubes' anodes, [high side drivers](https://electronics.stackexchange.com/questions/188745/high-side-driver-and-low-side-driver) are needed. To multiplex tubes' digits (cathodes), low side drivers are needed.

At the time of sourcing the components, no integrated IC driver was found for 160V high side switching. It's just too much voltage, so a discrete solution was implemented using high voltage transistors. Every tube anode required a pair of NPN-PNP transistors. For the digits multiplexing, discrete high voltage transistors were used.

![Tubes' multiplexing](/nixie_clock/assets/img/multiplexing.png "Tubes' Multiplexing")

An interesting addition were the resistors `R69` to `R77`. They're all the same high value, and they all join each low side transistor's drain terminal (or cathode terminal) with a floating common node for all digits. Why is that? The concept was taken from the reference design below [found here](https://www.pinitech.com/retrofit/schematics.php).

![bally7digit](/nixie_clock/assets/img/bally7digit.png)

Basically, __not all tubes' digits require the same voltage to glow__, thus, when multiplexing, it happens that one digit being addressed requires more anode-to-cathode voltage than others not being addressed, thus, the voltage applied to the one being addressed is enough to lightly make the one not being addressed glow, and the current flow goes throgh the low-side driver of the digit being addressed. This is because inside the tubes, no electrical isolation exists between the digits. This is the origin of the so-called ghosting effect in nixie tubes.

One solution is the one proposed: by tying all cathodes with high value resistors, only the addressed digit is fully ON, and all other cathodes are at a higher potential, that inhibits them from glowing. The solution is not perfect, but is simple and greatly reduces ghosting. Other solutions require more resistors, capacitors, zeners and the like, operating at high voltage, which requires way more space than what is available on the PCB (due to [clearance and creepage distances](https://www.smps.us/pcbtracespacing.html)).

## Timing

Each tube's anode is addressed for 5ms. During that time, the addressed tube's digit is ON for 1 to 5ms. It depends on the fade level. That way, individual fading levels can be applied to each tube. This is useful for implementing animations or transition effects.

Timing is completely controlled by ISRs. More on that, [here](/nixie_clock/docs/isr/).
