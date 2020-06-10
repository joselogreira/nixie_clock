
# **The Nixie Tube Clock** by [Nuvitron]

![alt text][clock_pcb]

At Nuvitron, I led the electronics development of The Nixie Clock hardware included in some of their models. This repository starts from the production version of the clock circuit, and includes further improvements in the form of:

- Code refactoring
- Hardware tweaks (pending)

---

## Toolchain:

This project is based on an [ATmega324pb][324pb] MCU from Atmel (now Microchip). The standard AVR toolchain for Ubuntu Linux was used. No bootloader implemented. Not tested under Windows or Mac.

- [avr-gcc] for compiling and building.
- [avr-libc] for AVR libraries.
- [avrdude] for programming, using [usbasp] programmer or alternatively the official [AVR ISP MK2][mk2]

There're many guides on how to install and set-up the toolchain. [Here's one] of many guides available.

***Sidenote:*** The ATmega324pb MCU is NOT supported by default, neither by *AVRdude* nor by *avr-gcc*, thus, a few extra steps need to be performed in order to get this toolchain to work properly. It's just a matter of including some extra files in the proper locatios to fully support the MCU:
* **AVRdude**: it uses a configuration file that includes all MCUs recognized by the program, among other things. Mine is at `/etc/avrdude.conf`. Edit it (with *root* proviledges) and add an entry for the *ATmega324pb*. Use the *ATmega324pa* as an example, which inherits most of the properties from the ATmega324p. The signature for the 324pb is `1e 95 17`. Alternatively, I've added my `avrdude.conf` to the repo, [here][special].
* **avr-gcc**: A device specification file must be included. My path: `/usr/lib/gcc/avr/5.4.0/device-spec/`. The missing file is called `specs-atmega324pb` and I found it in a Windows installation of [Atmel Studio][astudio], the Atmel IDE for Windows. Within this file there's a couple of dependencies pointing to files in the avr-libc path.
* **avr-libc**: Three files are needed. Again, I found them in the Atmel Studio Windows installation path. My paths for these files are:
  * `/usr/lib/avr/include/avr/` must include the file `iom324pb.h`, which is the memory mapping for all MCU peripherals and special registers.
  * `/usr/lib/avr/lib/avr5/` must include `crtatmega324pb.o` and `libatmega324pb.a`.

***Sidenote 2:*** The latest version of Atmel Studio (7.0, at the time of this writting) doesn't include support for the *ATmega324pb* out-of-the-box. Instead, the user must install an additional package to fully support it. It's called *"Device Firmware Pack"*, and it's installed within Atmel Studio in the *"Device Package Manager"* window. Search for ATmega_DFP and install it. Mine was version `1.2.272`. Once installed, the previously mentioned Windows files can be found at:
  * `\<install-path>\Atmel\Studio\7.0\packs\atmel\ATmega_DFP\1.2.272\include\avr\iom324pb.h`
  * `\<install-path>\Atmel\Studio\7.0\packs\atmel\ATmega_DFP\1.2.272\gcc\dev\atmega324pb\avr5\crtatmega324pb.o` and `libatmega324pb.a`
  * `\<install-path>\Atmel\Studio\7.0\packs\atmel\ATmega_DFP\1.2.272\gcc\dev\atmega324pb\device-specs\specs-atmega324pb`

For the sake of completeness, these files are also included [here][special].

---

## Project Folders
- **src**: contains all the code files needed, including library files.
- **special**: contains all *"additional"* files to support the MCU.
- **output**: not included in the repo. Automatically created once the makefile recipe is executed.

---

## Hardware Characteristics:
- MCU: ATmega324pb 
- (pending)

---

## License

Released under the [MIT License][license] - 2020 Jose Logreira

[clock_pcb]: https://raw.githubusercontent.com/joselogreira/nixie_clock/master/img/clock_pcb.png "Nixie Clock Circuit"
[Nuvitron]: <https://nuvitron.com>
[avr-gcc]: <https://www.microchip.com/mplab/avr-support/avr-and-arm-toolchains-c-compilers>
[avr-libc]: <https://www.nongnu.org/avr-libc/user-manual/overview.html>
[avrdude]: <https://www.nongnu.org/avrdude/>
[usbasp]: <https://www.fischl.de/usbasp/>
[Here's one]: <http://maxembedded.com/2015/06/setting-up-avr-gcc-toolchain-on-linux-and-mac-os-x/>
[special]: /special/
[astudio]: https://www.microchip.com/mplab/avr-support/atmel-studio-7
[324pb]: http://ww1.microchip.com/downloads/en/DeviceDoc/40001908A.pdf
[mk2]: http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-42093-AVR-ISP-mkII_UserGuide.pdf
[license]: /LICENSE