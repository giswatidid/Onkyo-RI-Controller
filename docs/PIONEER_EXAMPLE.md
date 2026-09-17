# Pioneer IR remote worked example

This document records the Pioneer remote integration used by the supplied example firmware.

It is an **input example**, not a requirement of Onkyo RI. The RI transmitter and Onkyo command set can be driven from another IR remote, buttons, serial commands, network control, or another source.

## Remote behaviour

The example uses a Pioneer remote associated with a VSX-534 receiver.

The remote frequently sends **paired IR frames** for a single button press. The firmware prints every decoded raw frame to Serial Monitor at 115200 baud, making it straightforward to identify the distinguishing frame for each function.

## Source codes

| Button | Distinguishing raw code | Controller action |
|---|---:|---|
| CD | `0xB34C5AA5` | Select CD mode and send C-705TX power-on `0xF04` |
| AUX | `0xFB045AA5` | Select tape mode and send K-505X wake `0xD15` |
| TAPE | `0xF30C5AA5` | Known code; not used by the supplied firmware |

The supplied firmware uses AUX as the tape-mode source trigger.

## Transport codes

| Pioneer button | Raw code |
|---|---:|
| PLAY | `0x3FC05AA5` |
| PAUSE | `0x3EC15AA5` |
| FF | `0xD12E50AF` |
| REW | `0xD22D50AF` |
| BACK | `0xF6095AA5` |

## Paired/common frames observed

Frames observed as paired/common traffic include:

```text
0x68975AA5
0xA8575AA5
0xA25D5AA5
```

For AUX, repeated single presses produced:

```text
IR: 0x649B5AA5
IR: 0xFB045AA5
```

The distinguishing AUX frame used by the controller is `0xFB045AA5`.

## Duplicate filtering

The example firmware uses a 300 ms duplicate filter:

```cpp
const unsigned long DEBOUNCE_MS = 300;
```

This has been tested with the Pioneer remote and prevents repeated accepted frames from triggering the same action twice within that interval.

## Mode mapping

### CD mode

Pressing CD:

1. sets `currentMode = MODE_CD`
2. sends `0xF04` to the C-705TX

While in CD mode:

| Remote button | Onkyo action |
|---|---|
| PLAY | CD play `0xF1B` |
| PAUSE | CD pause `0xF1F` |
| FF | Next track `0xF1D` |
| REW | Previous track `0xF1E` |

### Tape mode

Pressing AUX:

1. sets `currentMode = MODE_TAPE`
2. sends `0xD15` once as a K-505X wake command
3. sets `tapePlaying = false`

While in tape mode:

| Remote button | Onkyo action |
|---|---|
| PLAY | `0xD15` forward or `0xD16` reverse according to remembered direction |
| PAUSE | Stop `0xD13` |
| FF | Fast forward `0xD19` |
| REW | Rewind `0xD1A` |
| BACK | Toggle remembered forward/reverse direction |

If BACK is pressed while `tapePlaying == true`, the controller immediately sends the appropriate play command for the newly selected direction.

## Capturing a different IR remote

The supplied firmware prints the raw decoded value before its command-matching logic:

```cpp
uint32_t code = IrReceiver.decodedIRData.decodedRawData;

Serial.print("IR: 0x");
Serial.println(code, HEX);
```

To adapt the project to another IR remote:

1. Open Serial Monitor at 115200 baud.
2. Press the desired button once.
3. Record all frames printed for that single press.
4. Repeat after a short pause.
5. Identify the repeatable frame that distinguishes that function from common/paired frames.
6. Replace the relevant input-code constants and mode logic.
7. Leave the RI timing and Onkyo command values unchanged unless intentionally supporting different Onkyo equipment.