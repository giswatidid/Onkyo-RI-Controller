# Onkyo RI Controller

Arduino/ATmega328P controller and documentation for the Onkyo **RI (Remote Interactive)** control interface, including tested RI timing, command codes, wiring, build instructions, and an example IR-remote-to-RI controller.

The documented implementation has been tested with an **Onkyo C-705TX CD player** and **Onkyo K-505X cassette deck**. The project is centred on **Onkyo RI**, not on any particular source remote; the included Pioneer implementation is simply a worked example of translating IR commands into RI commands.

## What the controller does

The tested installation is:

```text
IR remote
    |
    v
38 kHz IR receiver
    |
    v
ATmega328P Pro Mini 5 V / 16 MHz
    |
    v
Onkyo RI
    |
    v
Onkyo C-705TX
    |
    v
Onkyo K-505X
```

The microcontroller receives an IR command, determines the active control mode, and generates the corresponding 12-bit RI waveform.

For the tested C-705TX/K-505X system, the controller's RI output is connected to the **C-705TX**, with the K-505X downstream through the C-705TX RI connection. The C-705TX continues to pass RI to the K-505X while powered off.

> Direct controller-to-K-505X operation was not successful in testing, so the documented known-good configuration is controller -> C-705TX -> K-505X. This is a model-specific test result, not a general requirement for every Onkyo RI device.

## Hardware

- ATmega328P Arduino Pro Mini-compatible board, 5 V / 16 MHz
- Generic 38 kHz demodulating IR receiver (1838/TL1838/VS1838B-style)
- 100 nF ceramic decoupling capacitor at the IR receiver
- 1 kΩ resistor in series with the RI signal
- Mono 3.5 mm cable for RI
- Regulated 5 V power source
- USB-C power-only breakout for the documented permanent build
- FT232RL/YP-05-style USB-to-TTL adapter for programming

## Repository contents

- [`docs/BUILD.md`](docs/BUILD.md) - parts, wiring, assembly, programming and test procedure
- [`docs/RI_PROTOCOL.md`](docs/RI_PROTOCOL.md) - tested RI waveform and transmitter behaviour
- [`docs/COMMANDS.md`](docs/COMMANDS.md) - tested C-705TX and K-505X RI commands
- [`docs/PIONEER_EXAMPLE.md`](docs/PIONEER_EXAMPLE.md) - worked Pioneer remote integration and captured IR codes
- [`docs/TROUBLESHOOTING.md`](docs/TROUBLESHOOTING.md) - practical diagnostic checks
- [`firmware/pioneer_ir_to_onkyo_ri/pioneer_ir_to_onkyo_ri.ino`](firmware/pioneer_ir_to_onkyo_ri/pioneer_ir_to_onkyo_ri.ino) - complete working example firmware

## RI electrical connection

```text
ATmega328P D8 ---- 1 kΩ ---- RI plug TIP
ATmega328P GND ------------- RI plug SLEEVE
```

Use a continuity test to identify the tip and sleeve conductors of your cable rather than relying on wire colours.

The tested implementation drives the RI line directly HIGH/LOW from the GPIO through the 1 kΩ series resistor.

## RI waveform

The tested transmission is a 12-bit command, MSB first:

```text
Header:
  HIGH 3000 us
  LOW  1000 us

Each bit:
  HIGH 1000 us

  0 = LOW 1000 us
  1 = LOW 2000 us

Trailer:
  HIGH 1000 us
  then LOW
```

See [`docs/RI_PROTOCOL.md`](docs/RI_PROTOCOL.md) for implementation details.

## Tested commands

### C-705TX CD player

| Function | RI command |
|---|---:|
| Power on | `0xF04` |
| Play | `0xF1B` |
| Pause | `0xF1F` |
| Next track | `0xF1D` |
| Previous track | `0xF1E` |

`0xF00` and `0xF01` were also observed as CD search commands. The example firmware deliberately maps FF/REW to next/previous track instead.

### K-505X cassette deck

| Function | RI command |
|---|---:|
| Stop | `0xD13` |
| Forward play | `0xD15` |
| Reverse play | `0xD16` |
| Fast forward | `0xD19` |
| Rewind | `0xD1A` |

See [`docs/COMMANDS.md`](docs/COMMANDS.md) for device-specific behaviour, including K-505X wake behaviour.

## Software requirements

The example firmware uses the Arduino environment and:

```cpp
#include <IRremote.hpp>
```

Install the **Arduino-IRremote** library before compiling.

For the tested Pro Mini:

- Board: **Arduino Pro or Pro Mini**
- Processor: **ATmega328P (5V, 16 MHz)**
- Serial Monitor: **115200 baud**

## Power

The documented build uses regulated USB 5 V connected to **VCC**, not RAW.

When programming with the YP-05/FT232-style adapter, do **not** power the board simultaneously from both the programming adapter and the permanent 5 V supply.

## Example remote integration

The included example firmware uses a Pioneer VSX-534 remote as the input device:

- CD selects CD-control mode and powers on the C-705TX.
- AUX selects tape-control mode and sends `0xD15` once to wake the K-505X.
- PLAY/PAUSE/FF/REW are translated according to the selected mode.
- BACK toggles the remembered tape direction.

The Pioneer-specific codes are isolated in the firmware and documented separately so another remote or input method can be substituted without changing the RI transmitter.

## Known limitation: cassette direction tracking

The controller remembers only the tape direction **it commanded**. The tested K-505X produced no detectable RI message when it automatically reversed at the end of a side. After an automatic reversal, the controller's remembered direction can therefore differ from the deck's actual direction.

## Status

The documented circuit and firmware are tested and working with the C-705TX/K-505X system, including:

- IR reception
- RI generation through the C-705TX -> K-505X chain
- C-705TX CD control
- K-505X transport control
- permanent regulated 5 V operation
- Pioneer IR input as the supplied example

See the build guide for the complete assembly procedure.