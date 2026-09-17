# Onkyo RI protocol notes

This document records the RI transmission method proven on the development system consisting of an ATmega328P Pro Mini, Onkyo C-705TX and Onkyo K-505X.

## Command format

The working RI command is **12 bits**, transmitted **MSB first**.

For example, a command such as:

```text
0xF04
```

is transmitted from bit 11 down to bit 0.

## Proven waveform

### Header

```text
HIGH approximately 3000 us
LOW  approximately 1000 us
```

### Each data bit

Every bit begins with:

```text
HIGH approximately 1000 us
```

The following LOW period determines the bit value:

```text
0 -> LOW approximately 1000 us
1 -> LOW approximately 2000 us
```

### Trailer

```text
HIGH approximately 1000 us
then LOW
```

## Timing summary

| Element | Level | Duration |
|---|---|---:|
| Header mark | HIGH | ~3000 us |
| Header space | LOW | ~1000 us |
| Bit mark | HIGH | ~1000 us |
| Zero space | LOW | ~1000 us |
| One space | LOW | ~2000 us |
| Trailer mark | HIGH | ~1000 us |
| Idle after frame | LOW | ongoing |

## Proven transmitter implementation

The working firmware uses ordinary GPIO HIGH/LOW signalling on D8:

```cpp
void sendRI(uint16_t command)
{
  noInterrupts();

  digitalWrite(RI_PIN, LOW);

  // Header
  digitalWrite(RI_PIN, HIGH);
  delayMicroseconds(3000);

  digitalWrite(RI_PIN, LOW);
  delayMicroseconds(1000);

  // 12 bits, MSB first
  for (int bit = 11; bit >= 0; bit--)
  {
    digitalWrite(RI_PIN, HIGH);
    delayMicroseconds(1000);

    digitalWrite(RI_PIN, LOW);

    if (command & (1 << bit))
      delayMicroseconds(2000);
    else
      delayMicroseconds(1000);
  }

  // Trailer
  digitalWrite(RI_PIN, HIGH);
  delayMicroseconds(1000);

  digitalWrite(RI_PIN, LOW);

  interrupts();
}
```

This implementation is proven reliable through the C-705TX -> K-505X chain.

## Interaction with IRremote

During development, IRremote interrupt activity interfered with reliable RI timing.

The working solution is to stop the IR receiver while sending RI:

```cpp
void sendOnkyo(uint16_t command)
{
  IrReceiver.stop();

  sendRI(command);

  IrReceiver.start();
}
```

`sendRI()` itself also disables interrupts while emitting the timing-critical frame.

The sequence above is part of the known-working implementation and should not be changed casually.

## Electrical connection used

The proven transmitter output is:

```text
ATmega328P D8 -> 1 kΩ -> RI TIP
ATmega328P GND -------> RI SLEEVE
```

The line idles LOW in the firmware.

No additional speculative output-stage redesign is required to reproduce the working development build.

## Scope of these notes

These timing and electrical notes describe what was measured and then proven to work on the development system. Onkyo used RI across many products and generations, so treat untested compatibility with other components as something to verify rather than assume.

When adapting the code to another Onkyo unit, keep the proven timing intact first and change only the command values or higher-level control logic needed for that device.