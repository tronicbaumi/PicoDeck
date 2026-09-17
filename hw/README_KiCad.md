# KiCad netlist export — `arduino_micro_panel.net`

This directory contains a **KiCad netlist-style export** describing the wiring
of the Arduino Micro control panel.

The `.net` file is a KiCad S-expression netlist (the same format KiCad's
"Export → Netlist" tool produces). It is **not** a schematic and does **not**
contain PCB layout — only the component list and the electrical nets (signal
connections between parts).

## Contents

| Reference | Part | Notes |
|-----------|------|-------|
| `U1`  | Arduino Micro (ATmega32u4)  | MCU, all digital pins used |
| `U2`  | SSD1306 0.96" OLED 128x64    | I2C, address 0x3C |
| `SW1`–`SW4` | Momentary pushbuttons | active-LOW with `INPUT_PULLUP` on D4–D7 |
| `ENC1` | Rotary encoder #1         | CLK=D8, DT=D9, SW=D12 |
| `ENC2` | Rotary encoder #2         | CLK=D10, DT=D11, SW=D13 |

## Nets

| Net | Connects |
|-----|----------|
| `+5V` | U1.5V → U2.VCC, ENC1.VCC, ENC2.VCC |
| `GND` | U1.GND → U2.GND, SW1–SW4 pin 1, ENC1.GND, ENC2.GND |
| `D2`  | U1.D2 → U2.SDA (OLED SDA) |
| `D3`  | U1.D3 → U2.SCL (OLED SCL) |
| `D4`  | U1.D4 → SW1.pin2 |
| `D5`  | U1.D5 → SW2.pin2 |
| `D6`  | U1.D6 → SW3.pin2 |
| `D7`  | U1.D7 → SW4.pin2 |
| `D8`  | U1.D8 → ENC1.CLK |
| `D9`  | U1.D9 → ENC1.DT |
| `D10` | U1.D10 → ENC2.CLK |
| `D11` | U1.D11 → ENC2.DT |
| `D12` | U1.D12 → ENC1.SW |
| `D13` | U1.D13 → ENC2.SW |

## How to use

1. Open KiCad's schematic editor (`Eeschema`).
2. Place parts matching the references above and wire them per the net table.
3. From the schematic, choose **Tools → Generate Netlist** to produce a fresh
   `.net` — or just keep this file as the canonical machine-readable export
   that is kept in sync with the firmware pin definitions in
   `../arduino_micro_panel.ino`.

The netlist is derived directly from the `#define` pin assignments in the
firmware, so it should be regenerated whenever those pins change.
