# Hardware build guide

This guide reproduces the proven ATmega328P-based Onkyo RI controller used with an Onkyo C-705TX and K-505X.

## 1. Parts

### Required

- Arduino Pro Mini-compatible board
  - ATmega328P
  - 5 V
  - 16 MHz
- 38 kHz demodulating IR receiver
  - the working build used a generic 1838/TL1838/VS1838B-style receiver
- 100 nF ceramic capacitor (`104` marking is common)
- 1 kΩ resistor
- Mono 3.5 mm plug/cable for Onkyo RI
- Regulated 5 V USB power source
- USB-C power-only breakout or equivalent regulated 5 V breakout
- Hook-up wire

### For programming

- FT232RL/YP-05-style USB-to-TTL serial adapter with DTR
- USB cable for the programming adapter
- Arduino IDE
- Arduino-IRremote library

## 2. Pin assignments

The proven firmware uses:

| Function | Pro Mini pin |
|---|---|
| IR receiver output | D2 |
| Onkyo RI output | D8 |
| Serial TX | TXO |
| Serial RX | RXI |
| Power | VCC |
| Ground | GND |

Do not change these pins unless you also intentionally change the firmware.

## 3. IR receiver wiring

The specific bare 38 kHz receiver used in the successful build had the following pinout when looking directly at the dark/sensing face with the legs pointing down:

```text
LEFT    MIDDLE    RIGHT
 OUT      GND      VCC
```

Connections:

```text
IR OUT  -> Pro Mini D2
IR GND  -> common GND
IR VCC  -> 5 V / VCC
```

Connect a **100 nF ceramic capacitor** directly between the receiver's VCC and GND pins.

```text
5 V ----+---- IR VCC
        |
      100 nF
        |
GND ----+---- IR GND
```

### Pinout warning

Do not assume every 1838-labelled or clone receiver has the same pin order. The orientation above is the pinout of the receiver proven in this build. Verify your part if it differs.

## 4. RI wiring

The proven RI connection is:

```text
Pro Mini D8 -> 1 kΩ resistor -> RI TIP
Pro Mini GND ----------------> RI SLEEVE
```

For the development cable:

```text
GREEN  = TIP    = RI signal
YELLOW = SLEEVE = GND
```

Cable colours are not guaranteed. Use a multimeter continuity test to identify the tip and sleeve conductors on another cable.

### Proven device chain

The working installation is:

```text
Pro Mini RI output
      |
      v
Onkyo C-705TX
      |
      v
Onkyo K-505X
```

The C-705TX passes RI communication to the K-505X even while the C-705TX is powered off.

Direct microcontroller -> K-505X tests were unsuccessful during development, so the documented build deliberately uses the C-705TX as the first RI device in the chain.

## 5. Permanent power wiring

The final build uses a USB-C power-only breakout as a regulated 5 V source.

```text
USB-C VBUS -> Pro Mini VCC
USB-C GND  -> common GND
```

### Important

Feed regulated 5 V into **VCC**, not `RAW`.

The common ground joins:

- USB-C ground
- Pro Mini ground
- IR receiver ground
- RI sleeve/ground

## 6. Programming-adapter wiring

The proven YP-05/FT232-style connection is:

| YP-05 | Pro Mini |
|---|---|
| DTR | DTR |
| RX | TXO |
| TX | RXI |
| VCC 5 V | VCC |
| GND | GND |
| CTS | not connected |

RX and TX are crossed because each device's transmitter connects to the other device's receiver.

### Power safety while programming

**Do not connect the YP-05 5 V supply and the permanent USB-C 5 V supply at the same time.**

When programming:

1. Disconnect permanent USB-C power.
2. Connect the YP-05 wiring above.
3. Connect the YP-05 to the computer.
4. Upload the sketch.
5. Disconnect the YP-05 before returning to permanent USB-C power.

## 7. Arduino IDE settings

Use:

```text
Board:      Arduino Pro or Pro Mini
Processor:  ATmega328P (5V, 16 MHz)
Serial:     115200 baud
```

Install the **Arduino-IRremote** library. The example firmware includes:

```cpp
#include <IRremote.hpp>
```

The programming adapter's DTR connection provides automatic reset for uploading.

Some garbage characters may briefly appear in Serial Monitor around reset/boot before the normal startup banner. This occurred on the development setup and did not indicate a functional fault.

## 8. Firmware

Open:

[`firmware/pioneer_ir_to_onkyo_ri/pioneer_ir_to_onkyo_ri.ino`](../firmware/pioneer_ir_to_onkyo_ri/pioneer_ir_to_onkyo_ri.ino)

The example is the complete working controller used during development. It translates a Pioneer remote into Onkyo RI commands.

The RI transmitter itself is not Pioneer-specific; another input method can call the same `sendOnkyo()`/`sendRI()` path with the required RI command values.

## 9. First power-on test

Before plugging the RI lead into Onkyo equipment, check:

- no short between 5 V and GND
- IR receiver VCC is on 5 V
- IR receiver GND is on common ground
- IR output reaches D2
- D8 reaches the RI tip only through the 1 kΩ resistor
- RI sleeve is common ground
- USB 5 V is connected to VCC, not RAW

Then power the controller and open Serial Monitor at 115200 baud.

The example firmware should print a startup banner ending with:

```text
Initial tape direction = FORWARD
Ready.
```

Pressing remote buttons should print their raw decoded IR values.

## 10. Functional test sequence

With the proven Pioneer example firmware, test in this order.

### CD

1. Press CD.
2. Confirm Serial Monitor reports `CONTROL MODE = CD`.
3. Confirm `CD POWER ON -> 0xF04`.
4. Test PLAY.
5. Test PAUSE.
6. Test FF as next track.
7. Test REW as previous track.

### Tape

1. Press AUX.
2. Confirm Serial Monitor reports `CONTROL MODE = TAPE`.
3. Confirm `TAPE WAKE -> 0xD15`.
4. Test PLAY.
5. Test PAUSE/STOP.
6. Test FF.
7. Test REW.
8. Test BACK to switch remembered forward/reverse direction.

## 11. K-505X wake behaviour

The following behaviour was observed on the development deck:

```text
Deck OFF + D13 -> no visible action
Deck OFF + D15 -> deck wakes/powers on, tape does not start moving
Deck ON  + D15 -> forward playback starts
Playing  + D15 -> essentially no change
```

For that reason, entering tape mode sends `0xD15` once as a wake command but then sets the firmware's `tapePlaying` flag to `false`.

## 12. Final installation

Once the system is tested with the programming adapter:

1. Disconnect the programming adapter.
2. Power the Pro Mini from the permanent USB-C 5 V breakout.
3. Confirm the startup and IR receiver still work.
4. Reconnect the RI cable to the C-705TX chain.
5. Repeat the CD and tape functional tests.

The development device has been proven working in this permanent USB-C-powered configuration.