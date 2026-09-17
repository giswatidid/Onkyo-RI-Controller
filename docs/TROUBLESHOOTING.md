# Troubleshooting

This guide focuses on faults actually encountered or directly relevant to the proven build.

## Serial Monitor shows garbage at reset

A few garbage characters may appear around reset/boot before the normal startup banner.

Use:

```text
115200 baud
```

If the normal banner follows and the controller operates correctly, the brief garbage seen on the development setup was not a fault.

## No startup banner at all

Check:

- Serial Monitor is set to 115200 baud.
- YP-05/FT232 `RX` is connected to Pro Mini `TXO`.
- YP-05/FT232 `TX` is connected to Pro Mini `RXI`.
- Grounds are common.
- The Pro Mini is receiving 5 V at VCC.
- The board/processor selection is `Arduino Pro or Pro Mini` / `ATmega328P (5V, 16 MHz)`.

## Upload fails

Check the programming wiring:

```text
YP-05 DTR -> Pro Mini DTR
YP-05 RX  -> Pro Mini TXO
YP-05 TX  -> Pro Mini RXI
YP-05 VCC -> Pro Mini VCC
YP-05 GND -> Pro Mini GND
CTS       -> unused
```

Also make sure the permanent USB-C supply is disconnected while the YP-05 is providing 5 V.

## IR buttons produce no serial output

Check:

- receiver OUT -> D2
- receiver GND -> common GND
- receiver VCC -> 5 V
- 100 nF capacitor is across receiver VCC/GND
- the receiver pinout matches the physical part

For the receiver proven in the build, viewed from the dark/sensing face with legs down:

```text
LEFT = OUT
MIDDLE = GND
RIGHT = VCC
```

Do not assume a replacement receiver has the same pinout without checking it.

## IR receiver becomes unresponsive or appears shorted

During permanent construction, one IR receiver ended up effectively shorted between **OUT and GND** and had to be replaced.

If a previously working circuit suddenly stops receiving IR after soldering:

1. Disconnect power.
2. Inspect solder bridges and wiring around the receiver.
3. Measure resistance/continuity between OUT and GND.
4. Compare against a known-good replacement if available.
5. Replace the receiver if it has been damaged.

The replacement receiver in the development unit restored normal operation immediately.

## IR is decoded, but Onkyo equipment does nothing

Check the RI path first:

```text
D8 -> 1 kΩ -> RI TIP
GND --------> RI SLEEVE
```

Also verify:

- the 3.5 mm cable is mono/connected as expected
- tip and sleeve were identified by continuity, not assumed from wire colour
- the controller and RI sleeve share common ground
- the correct device chain is being used

The proven chain is:

```text
controller -> C-705TX -> K-505X
```

Direct controller -> K-505X tests were unsuccessful on the development system.

## RI works unreliably when IRremote is active

This was encountered during development.

IRremote interrupt activity disturbed RI timing. The proven firmware deliberately does this:

```cpp
IrReceiver.stop();
sendRI(command);
IrReceiver.start();
```

`sendRI()` also brackets the timing-sensitive waveform with:

```cpp
noInterrupts();
...
interrupts();
```

Do not remove this behaviour as a cleanup step if you want to reproduce the known-working implementation.

## CD works but tape does not

Confirm the physical RI route is through the C-705TX before the K-505X.

Then test the known K-505X commands:

```text
0xD13 = stop
0xD15 = forward play / wake when off
0xD16 = reverse play
0xD19 = fast forward
0xD1A = rewind
```

If the deck is off, `0xD15` should wake the development K-505X without moving the tape. `0xD13` did not wake it.

## Selecting tape mode wakes the deck but does not start playback

That is expected in the proven implementation.

When the K-505X is off:

```text
D15 -> wakes deck, tape does not move
```

The firmware therefore treats the source-selection `D15` as a wake command and leaves:

```cpp
tapePlaying = false;
```

Press PLAY after the deck is awake to begin playback.

## Tape plays in the wrong remembered direction

The firmware can only remember the direction it last commanded.

The development K-505X emitted no detectable RI event when it automatically reversed at the end of a side. After such an automatic reversal, the controller's remembered direction may be opposite to the deck's actual physical direction.

Use BACK to toggle the controller's remembered direction.

## A remote button prints two different IR codes

This is normal for the tested Pioneer remote. Many buttons send paired frames.

For example, AUX repeatedly produced:

```text
IR: 0x649B5AA5
IR: 0xFB045AA5
```

The distinguishing AUX frame is `0xFB045AA5`.

When adapting another button, capture multiple single presses and identify the repeatable frame that distinguishes that function rather than assuming the first frame is the useful one.

## One press triggers an action twice

The example firmware has a 300 ms duplicate filter:

```cpp
const unsigned long DEBOUNCE_MS = 300;
```

This value is proven with the development Pioneer remote. If you are using the supplied remote mapping, leave it unchanged.

## Old Pioneer TAPE button no longer selects tape mode

This is intentional in the current example firmware.

The earlier distinguishing TAPE code was:

```text
0xF30C5AA5
```

The current installation uses AUX instead:

```text
0xFB045AA5
```

The old TAPE frame is not matched, so it has no controller action.

## Permanent power works differently from programming power

The permanent build uses regulated USB 5 V:

```text
USB-C VBUS -> Pro Mini VCC
USB-C GND  -> common GND
```

Do not feed regulated 5 V into RAW.

Do not connect both the YP-05 5 V output and permanent USB-C VBUS simultaneously.

## Quick known-good checklist

For the exact development configuration:

```text
MCU:        ATmega328P Pro Mini, 5 V / 16 MHz
IR input:   D2
RI output:  D8 through 1 kΩ to RI tip
RI ground:  common GND to RI sleeve
IR decoup:  100 nF VCC-GND at receiver
Serial:     115200
RI route:   controller -> C-705TX -> K-505X
Tape wake:  0xD15
```

If the hardware matches this and Serial Monitor shows the expected IR/action messages, troubleshoot the RI cable and device chain before changing the proven timing code.