# Pioneer IR remote worked example

This document records the specific IR integration used in the original working controller.

It is an **example input implementation**, not a requirement of Onkyo RI. The RI transmitter and Onkyo command set can be driven from another IR remote, buttons, serial commands, network control, or another source.

## Development remote

The example uses a Pioneer remote associated with a VSX-534 receiver.

The remote frequently sends **paired IR frames** for a single button press. The controller uses the distinguishing frame and ignores the other frame unless it has a purpose of its own.

The firmware prints every decoded raw frame to Serial Monitor at 115200 baud, which makes it straightforward to capture codes for another remote.

## Proven source codes

| Button | Distinguishing raw code | Controller action |
|---|---:|---|
| CD | `0xB34C5AA5` | Select CD mode and send C-705TX power-on `0xF04` |
| AUX | `0xFB045AA5` | Select tape mode and send K-505X wake `0xD15` |

An earlier version used the Pioneer TAPE button:

```text
TAPE = 0xF30C5AA5
```

The current firmware intentionally does **not** use that code. AUX is used instead because the cassette deck's analogue audio input is assigned to AUX on the VSX-534.

Pressing the old TAPE source button therefore has no controller action in the current firmware, although its decoded IR frames can still appear in Serial Monitor.

## Transport codes

| Pioneer button | Raw code |
|---|---:|
| PLAY | `0x3FC05AA5` |
| PAUSE | `0x3EC15AA5` |
| FF | `0xD12E50AF` |
| REW | `0xD22D50AF` |
| BACK | `0xF6095AA5` |

The PAUSE value is specifically:

```text
0x3EC15AA5
```

including the final `5`.

## Paired/common frames observed

Frames observed as paired/common traffic include:

```text
0x68975AA5
0xA8575AA5
0xA25D5AA5
```

For the AUX button, repeated single presses produced:

```text
IR: 0x649B5AA5
IR: 0xFB045AA5
```

The second frame, `0xFB045AA5`, is the distinguishing AUX frame used by the controller.

## Duplicate filtering

The example firmware has a 300 ms duplicate filter:

```cpp
const unsigned long DEBOUNCE_MS = 300;
```

This has been tested with the Pioneer remote and prevents a repeated copy of an accepted frame from triggering the same action again within that window.

## Mode mapping

### CD mode

Pressing CD:

1. sets `currentMode = MODE_CD`
2. sends `0xF04` to turn on/select the C-705TX

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

## Why AUX is used in the current installation

The TV is connected to the Pioneer VSX-534 using ARC/eARC. The cassette deck supplies analogue RCA audio.

The receiver configuration is:

```text
TV  -> ARC/eARC television audio
AUX -> cassette deck analogue RCA audio
```

This avoids assigning cassette analogue audio to the TV source and therefore avoids disturbing the TV input's ARC/eARC audio selection.

The result is that one AUX press performs both roles:

- the Pioneer receiver selects the cassette audio input
- the IR-to-RI controller enters tape mode and wakes the K-505X

## Capturing a different IR remote

The supplied firmware prints the raw decoded value before its command matching logic:

```cpp
uint32_t code = IrReceiver.decodedIRData.decodedRawData;

Serial.print("IR: 0x");
Serial.println(code, HEX);
```

To adapt the project to another IR remote:

1. Open Serial Monitor at 115200 baud.
2. Press the desired button once.
3. Record all frames printed for that single press.
4. Repeat the press after a short pause.
5. Identify the repeatable frame that distinguishes that button from common/paired frames.
6. Replace only the relevant `PIONEER_*` constants and mode logic as required.
7. Leave the proven RI timing and Onkyo command values unchanged unless you are intentionally supporting different Onkyo equipment.

This capture-first approach avoids guessing IR codes.