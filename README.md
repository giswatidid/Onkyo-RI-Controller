# Onkyo RI Controller

Arduino/ATmega328P controller and documentation for the Onkyo **RI (Remote Interactive)** control interface, including proven RI timing, command codes, wiring, build instructions, and an example IR-remote-to-RI controller.

This repository grew from a working adapter built to control an **Onkyo C-705TX CD player** and **Onkyo K-505X cassette deck** using a separate infrared remote. The RI signalling and hardware documented here have been tested on that equipment.

The project is deliberately centred on **Onkyo RI**, not on any particular source remote. The included Pioneer remote implementation is a worked example of how an IR remote can be translated into Onkyo RI commands.

## What the controller does

The tested installation is:

```text
Pioneer IR remote
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

The microcontroller receives an IR command, decides which Onkyo device/control mode is active, and generates the corresponding 12-bit RI waveform.

In the tested setup, the RI output is connected to the **C-705TX**, with the K-505X connected downstream through the C-705TX's RI connection. This path works even when the C-705TX itself is powered off.

> **Important:** Direct microcontroller-to-K-505X experiments were unsuccessful in this particular setup. The proven configuration is microcontroller -> C-705TX -> K-505X. This is a tested-system observation, not a claim that every K-505X or every Onkyo RI installation must be wired this way.

## Proven hardware

- ATmega328P Arduino Pro Mini-compatible board
- 5 V / 16 MHz
- Generic 38 kHz demodulating IR receiver (1838/TL1838/VS1838B-style unit used in the build)
- 100 nF ceramic decoupling capacitor at the IR receiver
- 1 kΩ resistor in series with the RI signal
- Mono 3.5 mm cable for RI
- USB-C 5 V power-only breakout for permanent power
- FT232RL/YP-05-style USB-to-TTL adapter for programming

## Repository contents

- [`docs/BUILD.md`](docs/BUILD.md) - complete parts, wiring, assembly, programming and test procedure
- [`docs/RI_PROTOCOL.md`](docs/RI_PROTOCOL.md) - proven RI waveform and transmitter behaviour
- [`docs/COMMANDS.md`](docs/COMMANDS.md) - proven C-705TX and K-505X RI commands
- [`docs/PIONEER_EXAMPLE.md`](docs/PIONEER_EXAMPLE.md) - worked Pioneer VSX-534 remote integration and captured IR codes
- [`docs/TROUBLESHOOTING.md`](docs/TROUBLESHOOTING.md) - faults encountered during the build and diagnostic checks
- [`firmware/pioneer_ir_to_onkyo_ri/pioneer_ir_to_onkyo_ri.ino`](firmware/pioneer_ir_to_onkyo_ri/pioneer_ir_to_onkyo_ri.ino) - complete proven example firmware

## RI electrical connection used in the working build

```text
ATmega328P D8 ---- 1 kΩ ---- RI plug TIP
ATmega328P GND ------------- RI plug SLEEVE
```

For the particular RI cable used during development:

- **green wire = TIP = RI signal**
- **yellow wire = SLEEVE = ground**

Do not assume cable colours are standard. Verify tip and sleeve continuity on your own cable.

The current active HIGH/LOW GPIO implementation is proven reliable through the C-705TX chain. The project does not require a speculative open-drain redesign to reproduce the tested build.

## RI waveform

The proven transmission is a 12-bit command, MSB first:

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

See [`docs/RI_PROTOCOL.md`](docs/RI_PROTOCOL.md) for the implementation details.

## Proven commands

### C-705TX CD player

| Function | RI command |
|---|---:|
| Power on | `0xF04` |
| Play | `0xF1B` |
| Pause | `0xF1F` |
| Next track | `0xF1D` |
| Previous track | `0xF1E` |

`0xF00` and `0xF01` were also observed as CD search commands, but the included example intentionally maps remote FF/REW buttons to next/previous track instead.

### K-505X cassette deck

| Function | RI command |
|---|---:|
| Stop | `0xD13` |
| Forward play | `0xD15` |
| Reverse play | `0xD16` |
| Fast forward | `0xD19` |
| Rewind | `0xD1A` |

See [`docs/COMMANDS.md`](docs/COMMANDS.md) for device-specific behaviour, including the K-505X wake behaviour.

## Software requirements

The example firmware is written for the Arduino environment and uses:

```cpp
#include <IRremote.hpp>
```

Install the **Arduino-IRremote** library in Arduino IDE before compiling the example.

For the tested Pro Mini:

- Board: **Arduino Pro or Pro Mini**
- Processor: **ATmega328P (5V, 16 MHz)**
- Serial Monitor: **115200 baud**

## Important power rule

The Pro Mini is permanently powered from regulated USB 5 V through **VCC**, not RAW.

When programming with the YP-05/FT232-style adapter, do **not** have both the programming adapter's 5 V supply and the permanent USB-C 5 V supply connected at the same time.

## Example remote integration

The included firmware uses a Pioneer VSX-534 remote as the input device. In that example:

- CD selects CD-control mode and powers on the C-705TX.
- AUX selects tape-control mode and sends `0xD15` once to wake the K-505X.
- PLAY/PAUSE/FF/REW are translated according to the selected mode.
- BACK toggles the remembered tape direction.

The Pioneer-specific IR codes are isolated in the firmware and documented separately so the RI implementation can be adapted to another IR remote or another control source.

## Known limitation: cassette direction tracking

The controller remembers only the tape direction **it commanded**. The tested K-505X produced no detectable RI message when it automatically reversed at the end of a side. Therefore, after an automatic physical reversal, the controller's remembered direction can differ from the deck's actual direction.

This limitation is accepted in the example firmware.

## Status

The documented hand-built circuit and included firmware are electrically and functionally proven on the development system:

- IR reception works.
- RI generation works through the C-705TX -> K-505X chain.
- C-705TX CD commands work.
- K-505X transport commands work.
- The permanent USB-C-powered build works.
- The example AUX source trigger works for selecting/waking tape mode.

See the build guide to reproduce the device.