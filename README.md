# PicoDeck

A small USB control panel for **PicoScope 7**, built around an Arduino Micro
(ATmega32u4). It enumerates as a USB HID keyboard and turns two rotary
encoders and four push buttons into the keyboard shortcuts PicoScope uses
for amplitude, offset, filter, time base, trigger and pre-trigger. A 0.96"
128x64 OLED shows the active channel and the current encoder modes.

```
  +--------------------------------------------+
  |        [ OLED 128x64  "PicoDECK" ]         |   <- sloped top face
  |          CH 1            AMP     TIME      |
  |                                            |
  |        (ENC1)                (ENC2)        |
  |                                            |
  |   (BTN1)  (BTN2)  (BTN3)  (BTN4)           |
  +--------------------------------------------+
```

## Repository layout

| Folder | Content | State |
|--------|---------|-------|
| [`fw/PicoDeck/`](fw/PicoDeck) | Arduino firmware (`.ino` tabs + headers) | working |
| [`mec/PicoDeck/`](mec/PicoDeck) | OpenSCAD housing (`PicoDeck.scad`), OLED datasheet, helper libraries (BOSL2, MCAD, Write) | working |
| [`hw/`](hw) | Early KiCad / Fritzing descriptions (`.net`, `.fz`), empty KiCad project in `hw/PicoDeck/` | **outdated**, see [Known gaps](#known-gaps) |
| [`doc/`](doc) | Screenshots, PicoScope 7 / Pico logos, Eagle-style script exports (`bom.scr`, `pcb.scr`, `sch.scr`), title page | reference |
| [`sw/`](sw) | PC software | placeholder, empty |

## Hardware

### Parts

| Qty | Part | Notes |
|-----|------|-------|
| 1 | Arduino Micro (ATmega32u4, 5 V / 16 MHz) | native USB, `Keyboard` library |
| 1 | OLED 0.96" 128x64, SSD1306, I2C | 4 pin module, address `0x3C`, 27.3 x 27.3 mm |
| 2 | Rotary encoder with push button (KY-040 / EC11 style) | 8 mm mounting holes in the housing |
| 4 | Momentary push button | 8 mm mounting holes in the housing |
| 4 | M3 screws (about 8-10 mm) | fix the base plate |

### Pin assignment (as defined in `fw/PicoDeck/general.h`)

| Function | Arduino pin | Notes |
|----------|-------------|-------|
| OLED SDA | D2 | hardware I2C of the Micro |
| OLED SCL | D3 | hardware I2C of the Micro |
| ENC1 SW / DT / CLK | D4 / D5 / D6 | `INPUT_PULLUP`, active low |
| ENC2 SW / DT / CLK | D7 / D8 / D9 | `INPUT_PULLUP`, active low |
| BTN1 | D10 | `INPUT`, **external pull-up required**, active low |
| BTN2 | D14 | as above |
| BTN3 | D15 | as above |
| BTN4 | D16 | as above |

Power: 5 V and GND from the Micro to the OLED and encoder modules. All
switches connect between the pin and GND. The OLED reset pin is shared with
the Arduino reset (`OLED_RESET = -1`).

## Firmware

### Build

Board: **Arduino Micro** (`arduino:avr:micro`). Libraries (Library Manager):

- `Adafruit SSD1306` (pulls in `Adafruit GFX Library` and `Adafruit BusIO`)
- `Adafruit GFX Library`
- `Adafruit SleepyDog Library` (included, currently not used functionally)
- `Keyboard` and `Wire`/`SPI` ship with the Arduino AVR core

```
arduino-cli lib install "Adafruit SSD1306" "Adafruit GFX Library" "Adafruit SleepyDog Library"
arduino-cli compile --fqbn arduino:avr:micro fw/PicoDeck
arduino-cli upload  --fqbn arduino:avr:micro -p COMx fw/PicoDeck
```

The display type is selected in `general.h` (`OLED64_09` = 0.96" 128x64 is
the default; `OLED32` and `OLED64_13` are prepared but commented out).

### Source files

| File | Purpose |
|------|---------|
| `PicoDeck.ino` | `setup()` / `loop()`; polls encoders and buttons every pass, prints the encoder state to the serial port once per second (`MAIN_TIME`) |
| `general.h` | pin definitions, key assignments, display config, state variables, structs |
| `ENCODER.ino` | quadrature decoding (transition table, 4 sub-steps = 1 detent), switch debounce, mode switching, USB key output |
| `BUTTONS.ino` | button polling with debounce, key is typed on **release** |
| `OLED.ino` | display init (PicoScope 7 logo for 2 s) and the three redraw functions |
| `logo.h` | PicoScope 7 start-up bitmap |
| `GPIO.ino`, `LOGGER.ino` | empty placeholders |

Debounce time is 25 ms (`SW_DEBOUNCE_MS`). Serial debug output runs at
115200 baud.

### Operation

The deck keeps a current **channel** (CH 1..4) and, per channel, an encoder 1
mode. Encoder 2 has one global mode.

**Buttons** (key sent on release)

| Button | Pin | Action |
|--------|-----|--------|
| BTN1 | D10 | `Space` |
| BTN2 | D14 | `2` |
| BTN3 | D15 | cycles the channel CH 1 -> 2 -> 3 -> 4 -> 1 (no key sent) |
| BTN4 | D16 | `X` |

**Encoder 1** - channel settings. Pressing its switch cycles the mode of the
*current channel*: `AMP` -> `OFF` -> `FLT` -> `AMP`.

| Channel | AMP: CW / CCW | OFF (offset) | FLT (filter) |
|---------|---------------|--------------|--------------|
| CH 1 | `q` / `1` | `Shift` + key, directions swapped | `Alt` + key |
| CH 2 | `w` / `2` | as above | as above |
| CH 3 | `e` / `3` | as above | as above |
| CH 4 | `r` / `4` | as above | as above |

Entering and leaving `FLT` additionally sends `Alt` + `a` / `s` / `d` / `f`
for channel 1 / 2 / 3 / 4, which toggles the filter dialog in the same way
on both transitions.

**Encoder 2** - time base and trigger. Its switch cycles `TIME` -> `TRIG` ->
`PRE` -> `TIME`.

| Mode | CW | CCW |
|------|----|-----|
| TIME | `Up arrow` | `Down arrow` |
| TRIG (trigger level) | `b` | `v` |
| PRE (pre-trigger) | `m` | `n` |

The key tables are at the top of `general.h` (buttons, encoder fallback
keys) and in `ENCODER.ino` (`enc1CtrlCW/CCW`, `enc1FltEnter`, `Send_Rotation`).
Adjust them if your PicoScope shortcut set differs.

### Display

Text size 2 (12 x 16 px per character) on a 128 x 64 screen:

| Row (y) | Content |
|---------|---------|
| 0 | `PicoDECK` header, centered |
| 24 | `CH x` |
| 48 left | mode of encoder 1 on the current channel: `AMP` / `OFF` / `FLT` |
| 48 right | mode of encoder 2: `TIME` / `TRIG` / `PRE` |

After power-up the PicoScope 7 logo is shown for 2 seconds.

## Housing (OpenSCAD)

[`mec/PicoDeck/PicoDeck.scad`](mec/PicoDeck/PicoDeck.scad) generates a
wedge-shaped desktop housing in two parts: the **top shell** and a separate
**base plate**. Render with OpenSCAD (tested with the nightly build); no
external libraries are needed by this file.

Select what to render with the `part` variable at the top of the file:
`both` (side by side, default), `assembly`, `top`, or `bottom`.

### Main dimensions (all parameters at the top of the file)

| Parameter | Default | Meaning |
|-----------|---------|---------|
| `W` | 120 mm | width |
| `D` | 80 mm | depth |
| `Hf` / `Hb` | 20 / 60 mm | height front / back (slope about 26.6 degrees) |
| `r` | 3 mm | edge radius of the top shell (the base plate edges are not rounded) |
| `t` | 2.4 mm | wall thickness |

### Controls on the sloped face

Positions are measured along the slope, `s` = distance from the front edge.
Holes are perpendicular to the face.

| Element | Parameters | Default |
|---------|------------|---------|
| 4 push buttons, 8 mm | `btn_s`, `btn_pitch`, `btn_d` | s = 12 mm, 22 mm pitch, centered |
| 2 encoders, 8 mm | `enc_s`, `enc_pitch`, `enc_d` | s = 36 mm, 40 mm pitch, centered |
| OLED window | `oled_s`, `oled_win`, `oled_win_dy` | s = 66 mm, 22.4 x 11.6 mm, offset +1.76 mm |

### OLED mounting (from the datasheet in `mec/PicoDeck/`)

| Item | Datasheet | In the model |
|------|-----------|--------------|
| PCB | 27.3 x 27.3 mm (+-0.5) | pocket 28.0 x 28.0 mm (`oled_mod`) |
| Active area | 21.74 x 10.86 mm | window 22.4 x 11.6 mm (about 0.35 mm margin) |
| Active area position | 1.76 mm from PCB center towards the pin header | `oled_win_dy = 1.76` |
| Mounting holes | 4 x 2.5 mm, pitch 20.7 x 23.2 mm | 4 pegs 2.2 mm with 3.4 mm seats (`oled_hole`, `oled_peg_d`, `oled_boss_d`) |

Assumptions that are **not** in the datasheet: PCB 1.0 mm thick, glass stack
about 1.4 mm (`oled_pcb_t`, `oled_boss_h = 1.6`), module mounted with the pin
header towards the back of the housing. Rotating the module by 180 degrees
means `oled_win_dy = -1.76`.

### Assembly

- The base plate has a centering lip and **snap bumps** on all four sides that
  click into grooves on the inner wall of the top shell (`tol`, bump and
  groove sizes are tunable).
- Four **screw posts** hang from the roof of the shell and are connected to
  the side and front/back walls by 2 mm ribs. Pilot hole 2.6 mm for M3
  self-tapping screws; the base plate has 3.4 mm countersunk holes, screws
  go in from below.
- The base plate carries **4 standoffs** (6 mm diameter, 6 mm high, 2.6 mm
  blind pilot hole for M3) for a 70 x 50 mm prototyping PCB. The board is
  centered in x and sits against the back wall (3 mm gap); the holes are 3 mm
  from the board edges (`pcb_size`, `pcb_hole_in`, `pcb_post_d`, `pcb_post_h`,
  `pcb_screw_d`, `pcb_back`). Hole positions: x = 28 / 92 mm, y = 27.6 / 71.6 mm
  with the default housing. Check them against your board and adjust
  `pcb_hole_in` if its holes are placed differently.
- The back wall has a **USB-C opening** (13 x 7.5 mm, rounded) centered in x.
  Its height is computed for an Arduino Micro plugged into single-row
  female headers on the prototyping PCB: 6 mm standoff + 1.6 mm PCB + 8.5 mm
  header + 1.6 mm Micro PCB + half of the 3.2 mm connector = connector center
  at about z = 19.3 mm. Adjust `header_h`, `pcb_t`, `micro_t`, `usb_conn_h`,
  `usb_cut` and `usb_x` if your parts differ.
- Print the top shell open side down; the sloped roof needs supports. Print
  the base plate flat.
- `part = "assembly"` shows the closed housing for checking fit.

## Known gaps

- `hw/` still describes an earlier pin assignment (encoders on D8-D13,
  buttons on D4-D7 with internal pull-ups). The netlist (`arduino_micro_panel.net`),
  Fritzing description (`arduino_micro_panel.fz`) and the helper READMEs
  `hw/README_KiCad.md` / `hw/README_FZ.md` do **not** match the current firmware
  in `general.h`. The KiCad project in `hw/PicoDeck/` is an empty skeleton.
- The firmware expects external pull-ups on D10, D14, D15 and D16.
- The firmware uses polling in `loop()` (no pin-change interrupts), so very
  fast encoder turns can lose steps if the loop is delayed. The `Send_Combo`
  helper blocks for about 90 ms per modifier keystroke.
- `sw/` is a placeholder.

## License

See [LICENSE](LICENSE).
