# Troubleshooting

This guide covers practical checks for the documented controller.

## No startup banner

Check:

- Serial Monitor is set to 115200 baud.
- YP-05/FT232 `RX` is connected to Pro Mini `TXO`.
- YP-05/FT232 `TX` is connected to Pro Mini `RXI`.
- Grounds are common.
- The Pro Mini is receiving 5 V at VCC.
- Arduino IDE is set to `Arduino Pro or Pro Mini` / `ATmega328P (5V, 16 MHz)`.

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

Make sure the permanent 5 V supply is disconnected while the programming adapter is supplying power.

## IR buttons produce no serial output

Check:

- receiver OUT -> D2
- receiver GND -> common GND
- receiver VCC -> 5 V
- 100 nF capacitor is connected across receiver VCC/GND
- the receiver pinout matches the actual part

For the receiver used in the tested build, viewed from the dark/sensing face with legs down:

```text
LEFT = OUT
MIDDLE = GND
RIGHT = VCC
```

Do not assume another receiver has the same pinout without checking it.

## IR is decoded, but Onkyo equipment does nothing

Check the RI connection:

```text
D8 -> 1 kΩ -> RI TIP
GND --------> RI SLEEVE
```

Verify:

- tip and sleeve were identified by continuity
- the controller and RI sleeve share common ground
- the correct device chain is being used

For the tested C-705TX/K-505X setup, the known-good route is:

```text
controller -> C-705TX -> K-505X
```

## RI works unreliably when IRremote is active

The tested firmware deliberately stops IR reception while transmitting RI:

```cpp
IrReceiver.stop();
sendRI(command);
IrReceiver.start();
```

`sendRI()` also disables interrupts during the timing-sensitive waveform:

```cpp
noInterrupts();
...
interrupts();
```

Keep this behaviour when reproducing the tested implementation.

## CD works but tape does not

Confirm the physical RI route is through the C-705TX before the K-505X, then verify the known K-505X commands:

```text
0xD13 = stop
0xD15 = forward play / wake when off
0xD16 = reverse play
0xD19 = fast forward
0xD1A = rewind
```

On the tested K-505X, `0xD15` wakes the deck when it is off; `0xD13` does not.

## Selecting tape mode wakes the deck but does not start playback

This is expected with the supplied firmware.

When the tested K-505X is off:

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

The tested K-505X produced no detectable RI event when it automatically reversed at the end of a side. After an automatic reversal, the controller's remembered direction may therefore differ from the deck's actual direction.

Use BACK to toggle the remembered direction.

## A remote button prints two different IR codes

This is normal for the tested Pioneer remote. Many buttons send paired frames.

For example, AUX repeatedly produced:

```text
IR: 0x649B5AA5
IR: 0xFB045AA5
```

The distinguishing AUX frame is `0xFB045AA5`.

When adapting another remote, capture multiple single presses and identify the repeatable frame that distinguishes the required function rather than assuming the first frame is the useful one.

## One press triggers an action twice

The example firmware uses a 300 ms duplicate filter:

```cpp
const unsigned long DEBOUNCE_MS = 300;
```

This value is tested with the supplied Pioneer example.

## Power checks

The permanent build uses regulated 5 V:

```text
5 V -> Pro Mini VCC
GND -> common GND
```

Do not feed regulated 5 V into RAW.

Do not connect both the programming adapter's 5 V output and the permanent 5 V supply simultaneously.

## Known-good configuration

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

If the hardware matches this configuration and Serial Monitor shows the expected IR/action messages, check the RI cable and device chain before changing the tested RI timing.