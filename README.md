# Arduino Micro PICODeck

A small Arduino Micro (ATmega32u4 / Leonardo-compatible) sketch that wires up
two KY-040 rotary encoders (with built-in push buttons), four momentary push
buttons, and a 128x64 I2C SSD1306 OLED, and displays everything on-screen.

## Hardware

| Function           | Arduino pin |
|--------------------|-------------|
| OLED SDA           | D2          |
| OLED SCL           | D3          |
| OLED I2C address   | 0x3C        |
| ENC1 CLK (A)       | D8          |
| ENC1 DT  (B)       | D9          |
| ENC1 SW            | D12         |
| ENC2 CLK (A)       | D10         |
| ENC2 DT  (B)       | D11         |
| ENC2 SW            | D13         |
| SW1                | D4          |
| SW2                | D5          |
| SW3                | D6          |
| SW4                | D7          |

All switch inputs (ENC1_SW, ENC2_SW, SW1..SW4) and the rotary-encoder A/B
phases are wired between the pin and GND and use the AVR internal pull-ups
(`pinMode(..., INPUT_PULLUP)`). They are **active LOW** (LOW = pressed).

Power: 5V and GND from the Micro to the OLED VCC/GND and to one side of each
button. Pull-ups are internal so no external resistors are required.

## Library install (Arduino Library Manager)

1. `Adafruit SSD1306` (this also pulls in `Adafruit GFX Library` and
   `Adafruit BusIO` automatically).
2. `Adafruit GFX Library`

Both install through **Sketch → Include Library → Manage Libraries...** or
the CLI:

```
arduino-cli lib install "Adafruit SSD1306" "Adafruit GFX Library"
```

## Encoder reading strategy

On the ATmega32u4 (Micro / Leonardo) the Arduino pin → AVR port mapping is:

| Pin | Port bit | PCINT |
|-----|----------|-------|
| D8  | PB4      | PCINT4 |
| D9  | PB5      | PCINT5 |
| D10 | PB6      | PCINT6 |
| D11 | PB7      | PCINT7 |
| D12 | PD6      | —      |
| D13 | PC7      | —      |

Only **D8..D11** sit on a pin-change interrupt group (PCINT0 / PB). D12 and
D13 are NOT PCINT-capable on the 32u4, so the encoder push buttons on D12
and D13 are debounced + sampled in the main loop, alongside SW1..SW4.

The quadrature A/B phases on D8..D11 are captured by a small `PCINT0_vect`
ISR that snapshots `PINB` on every change and bumps per-encoder "edge"
counters (`enc1_edges`, `enc2_edges`, both `volatile`). The main loop drains
those counters and runs the Gray-code state machine via a transition table
(`qtrans`). This is the standard "ISR records edge, loop decodes" pattern:
the ISR stays tiny (no debounce, no I2C, no display), and everything that
needs timing slack runs without being held off.

If you swap D12/D13 for SW inputs and move the encoder SW lines to PCINT
pins, the ISR can be extended to cover them too. The sketch keeps that
extension one place (`pcint_init` / the ISR body) so the change is local.

Debounce for all six polled inputs (SW1..SW4 + ENC1_SW + ENC2_SW) is a
25 ms time-based filter on the polled `digitalRead()` value.

## Files

- `arduino_micro_panel.ino` — the sketch (verified to compile for
  `arduino:avr:micro` with `arduino-cli 1.5.1`, `arduino:avr@1.8.8`,
  `Adafruit_SSD1306@2.5.17`, `Adafruit_GFX@1.12.6`).
- `kicad/` — KiCad-style netlist description and a skeleton `.kicad_sch`
  README explaining how to import it.
- `fritzing/` — Fritzing-style XML description and a short README mapping
  the parts to real Fritzing parts bin entries.

## Notes / assumptions

- The Micro's hardware I2C pins are fixed (D2 = SDA, D3 = SCL on this
  board), so `Wire.begin()` with no arguments selects the right pins.
- The OLED reset pin is tied to MCU reset (`OLED_RST = -1`), which is the
  common case for the popular 0.96" SSD1306 breakout boards.
- Encoder count direction follows the KY-040 datasheet (CLK leads DT = CW).
  If your physical orientation reads the other way, flip the signs in
  `qtrans[]` or negate `encoder*_count` on each step.
- Display refresh is throttled to ~30 Hz to keep I2C traffic and OLED
  flicker low. The encoder ISR still runs at full speed.
