# Fritzing description — `arduino_micro_panel.fz.xml`

This directory contains a **description-level export** of the Arduino Micro
control panel wiring as a single XML file. It is **not** a full Fritzing
`.fzz` archive — it is a plain text XML description of parts and connections
that maps directly onto real Fritzing parts in the parts bin.

## What's in the file

`arduino_micro_panel.fz.xml` contains three sections:

1. **`<parts>`** — describes each module: Arduino Micro, SSD1306 OLED,
   generic pushbutton, and an EC11-style rotary encoder. Each part lists
   the connector names that are referenced in the wires section.
2. **`<instances>`** — declares which physical instances exist (U1, U2,
   SW1..SW4, ENC1, ENC2) and which part definition they use.
3. **`<wires>`** — every connection in the project, grouped by net
   (`+5V`, `GND`, `D2`, `D3`, `D4`..`D7`, `D8`..`D13`). Each wire
   references the connector IDs of two instances.
4. **`<labels>`** — human-readable net labels mirroring the README in
   the project root.

## Mapping to real Fritzing parts

To rebuild the diagram in Fritzing:

| XML `part` id          | Fritzing bin part (typical) |
|------------------------|-----------------------------|
| `part_arduino_micro`   | **Sparkfun → Arduino Pro Micro** (8 MHz/16 MHz, 5 V) |
| `part_ssd1306_oled`    | **Adafruit → SSD1306 128x64 OLED** (I2C variant) |
| `part_pushbutton`      | **Core → Switch → Momentary** (any 6 mm tactile) |
| `part_rotary_encoder`  | **Sparkfun → Rotary Encoder** (EC11-style) |

After placing the matching bin parts on the breadboard view, wire them
exactly as the `<wires>` section does. The connector names line up
1-to-1 with the pins of each bin part.

## Pin mapping reference

| Function           | Arduino pin |
|--------------------|-------------|
| OLED SDA           | D2          |
| OLED SCL           | D3          |
| OLED I2C address   | 0x3C        |
| ENC1 CLK / DT / SW | D8 / D9 / D12 |
| ENC2 CLK / DT / SW | D10 / D11 / D13 |
| SW1 / SW2 / SW3 / SW4 | D4 / D5 / D6 / D7 |

All inputs are configured as `INPUT_PULLUP` and read active-LOW; this is
already handled in `../arduino_micro_panel.ino`, so the schematic just
needs to show the switch/encoder pin going to the MCU pin (no external
pull-up resistor required).

## Notes

- This XML is descriptive, not executable Fritzing source. It is intended
  to make the wiring easy to redraw in Fritzing, or to consume from a
  script that generates a real `.fzz`.
- For the canonical PCB-level export, see the KiCad netlist in
  `../kicad/arduino_micro_panel.net`.
