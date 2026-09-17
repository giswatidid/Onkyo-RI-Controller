# Proven Onkyo RI commands

These commands were tested on the development equipment. They should be treated as a verified reference for the listed models, not as a complete catalogue of all Onkyo RI commands.

## Onkyo C-705TX CD player

| Function | Command | Status |
|---|---:|---|
| Power on | `0xF04` | Proven |
| Play | `0xF1B` | Proven |
| Pause | `0xF1F` | Proven |
| Next track | `0xF1D` | Proven |
| Previous track | `0xF1E` | Proven |
| Search command | `0xF00` | Observed |
| Search command | `0xF01` | Observed |

The example firmware intentionally maps the source remote's FF and REW buttons to `0xF1D` and `0xF1E` so they behave as next/previous track rather than CD search.

## Onkyo K-505X cassette deck

| Function | Command | Status |
|---|---:|---|
| Stop | `0xD13` | Proven |
| Forward play | `0xD15` | Proven |
| Reverse play | `0xD16` | Proven |
| Fast forward | `0xD19` | Proven |
| Rewind | `0xD1A` | Proven |

## K-505X wake behaviour

A useful device-specific behaviour was discovered during testing:

| Deck state | Command | Result |
|---|---:|---|
| Off | `0xD13` | Nothing visible |
| Off | `0xD15` | Wakes/powers the deck; tape does not move |
| On | `0xD15` | Starts forward playback |
| Already playing | `0xD15` | Essentially no change |

The example controller therefore sends `0xD15` once when tape mode is selected. It treats that transmission as a **wake command**, not proof that the tape is now playing.

## Tape direction tracking limitation

The example firmware remembers the last playback direction that it commanded:

```text
FORWARD <-> REVERSE
```

It uses:

- `0xD15` for forward play
- `0xD16` for reverse play

When BACK is pressed in tape mode, the remembered direction toggles. If the deck is already considered to be playing, the firmware immediately sends the newly selected direction command.

During development, the K-505X produced **no detectable RI message when it automatically reversed at the end of a tape side**. Therefore the controller cannot automatically synchronize its remembered direction after the deck performs an automatic physical reversal.

This is a known and accepted limitation of the example implementation.

## Device chain used for testing

The commands above were sent through:

```text
ATmega328P -> C-705TX -> K-505X
```

The C-705TX continued to pass RI through to the K-505X while the CD player itself was powered off.