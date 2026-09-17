/**
 * arduino_micro_panel.ino
 *
 * Arduino Micro (ATmega32u4 / Leonardo-compatible) control panel:
 *   - 1x I2C SSD1306 128x64 OLED  (SDA=D2, SCL=D3, addr 0x3C)
 *   - 2x KY-040 rotary encoders w/ push button (ENC1, ENC2)
 *       ENC1: CLK=D8,  DT=D9,  SW=D12
 *       ENC2: CLK=D10, DT=D11, SW=D13
 *   - 4x momentary push buttons   (SW1=D4, SW2=D5, SW3=D6, SW4=D7)
 *
 * All switch inputs are wired to GND and use INPUT_PULLUP (active LOW).
 *
 * Display shows: encoder counts, encoder-button states, and the 4 button states.
 * Buttons are debounced (~25 ms).
 *
 * --------------------------------------------------------------------
 *  Encoder reading strategy (honest about the ATmega32u4 pin map)
 * --------------------------------------------------------------------
 *  On the Micro the Arduino pin -> AVR port mapping (see
 *  variants/leonardo/pins_arduino.h) is:
 *
 *     D8  -> PB4  (PCINT4)    \
 *     D9  -> PB5  (PCINT5)     |  ENC1 A/B  -> PCINT0 group (PB4..PB7)
 *     D10 -> PB6  (PCINT6)     |  ENC2 A/B  -> PCINT0 group (PB4..PB7)
 *     D11 -> PB7  (PCINT7)    /
 *     D12 -> PD6  (no PCINT)      ENC1 SW  -> polled
 *     D13 -> PC7  (no PCINT)      ENC2 SW  -> polled
 *
 *  Only D8..D11 are on PCINT-capable pins (PCINT0 group).  D12 and D13
 *  are NOT PCINT pins on the ATmega32u4, so the encoder push buttons
 *  there are debounced + sampled in the main loop, alongside SW1..SW4.
 *
 *  The quadrature A/B phases for both encoders are captured by a single
 *  PCINT0 ISR that snapshots PINB on every change and bumps per-encoder
 *  "edge" counters; the main loop drains the counters and runs the
 *  Gray-code state machine.  This is the standard "ISR records edge,
 *  loop decodes" pattern: the ISR stays tiny (no debounce, no LCD),
 *  and all of the work that needs timing slack runs without interrupts
 *  being held off.
 *
 *  Debounce for SW1..SW4, ENC1_SW, ENC2_SW is a 25 ms time-based filter
 *  on the polled digitalRead() value.
 *
 *  This combination is reliable on the Micro and is documented inline
 *  so future maintainers don't have to re-derive it.
 *
 * Libraries (Arduino Library Manager):
 *   - Adafruit SSD1306
 *   - Adafruit GFX Library
 *
 * Compile / board: Arduino Micro, Tools > USB Type = "Serial" (HID off)
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* -------------------------------------------------------------------------- */
/*  Pin definitions                                                           */
/* -------------------------------------------------------------------------- */

// I2C OLED
#define OLED_SDA   2
#define OLED_SCL   3
#define OLED_ADDR  0x3C

// Rotary encoders
#define ENC1_CLK   8
#define ENC1_DT    9
#define ENC1_SW    12
#define ENC2_CLK   10
#define ENC2_DT    11
#define ENC2_SW    13

// Momentary buttons
#define SW1_PIN    4
#define SW2_PIN    5
#define SW3_PIN    6
#define SW4_PIN    7

// PB4..PB7 mask for PCINT0 group (D8..D11 = ENC1/ENC2 phases)
#define PB_ENC_MASK  0xF0   // 0b11110000

/* -------------------------------------------------------------------------- */
/*  Display                                                                   */
/* -------------------------------------------------------------------------- */

#define SCREEN_W   128
#define SCREEN_H   64
#define OLED_RST   -1       // reset tied to MCU reset on most breakouts

Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, OLED_RST);

/* -------------------------------------------------------------------------- */
/*  Quadrature state + PCINT edge counters                                    */
/* -------------------------------------------------------------------------- */

// Each ISR invocation increments one of these per encoder; the loop drains
// them by reading the current (CLK, DT) pair and consulting qtrans.
static volatile uint8_t enc1_edges = 0;     // pending edges for ENC1
static volatile uint8_t enc2_edges = 0;     // pending edges for ENC2
static volatile uint8_t portb_last = 0xFF;  // last-seen PINB snapshot

// Debounced encoder counts (signed)
static int32_t encoder1_count = 0;
static int32_t encoder2_count = 0;

// Quadrature state: low 2 bits hold the (CLK, DT) Gray-code sample
static uint8_t enc1_state = 0;
static uint8_t enc2_state = 0;

// Transition table: row = previous 2-bit state, col = current 2-bit state.
// Output: -1, 0, or +1. KY-040: CLK leads DT -> CW (per datasheet). Flip
// signs here if your physical orientation reads the other way around.
static const int8_t qtrans[4][4] = {
  // prev=00
  {  0, +1, -1,  0 },
  // prev=01
  { -1,  0,  0, +1 },
  // prev=10
  { +1,  0,  0, -1 },
  // prev=11
  {  0, -1, +1,  0 },
};

/* -------------------------------------------------------------------------- */
/*  Switch debouncing                                                         */
/* -------------------------------------------------------------------------- */

#define DEBOUNCE_MS  25
#define NUM_BTNS     6     // SW1..SW4 + ENC1_SW + ENC2_SW

static const uint8_t btn_pin[NUM_BTNS] = {
  SW1_PIN, SW2_PIN, SW3_PIN, SW4_PIN,
  ENC1_SW, ENC2_SW
};

// Current stable state (HIGH = released, LOW = pressed because INPUT_PULLUP)
static uint8_t  btn_state[NUM_BTNS];
static uint8_t  btn_last[NUM_BTNS];
static uint32_t btn_last_change[NUM_BTNS];

/* -------------------------------------------------------------------------- */
/*  Pin-Change Interrupt setup (ATmega32u4, PCINT0 group only)                */
/* -------------------------------------------------------------------------- */

ISR(PCINT0_vect) {
  uint8_t now = PINB;
  uint8_t changed = (now ^ portb_last) & PB_ENC_MASK;
  portb_last = now;
  if (!changed) return;

  // PB4..PB5 = ENC1, PB6..PB7 = ENC2.  Count one "edge" per changed bit;
  // the loop will read both phases and resolve the quadrature transition.
  if (changed & 0x30) enc1_edges++;   // PB4 | PB5 changed -> ENC1 edge
  if (changed & 0xC0) enc2_edges++;   // PB6 | PB7 changed -> ENC2 edge
}

static void pcint_init(void) {
  // Snapshot current port state before enabling the interrupt so the very
  // first transition is detected (not missed as a phantom edge).
  portb_last = PINB;

  // Pull-ups on the encoder phases (in addition to INPUT_PULLUP set below)
  pinMode(ENC1_CLK, INPUT_PULLUP);
  pinMode(ENC1_DT,  INPUT_PULLUP);
  pinMode(ENC2_CLK, INPUT_PULLUP);
  pinMode(ENC2_DT,  INPUT_PULLUP);

  // Enable PCINT4..PCINT7 (PB4..PB7) in the PCMSK0 mask register.
  PCMSK0 |= (1 << PCINT4) | (1 << PCINT5) | (1 << PCINT6) | (1 << PCINT7);

  // Enable the PCINT0 interrupt group (only one PCINT group exists on
  // ATmega32u4, so there's nothing else to turn on).
  PCICR  |= (1 << PCIE0);
}

/* -------------------------------------------------------------------------- */
/*  Button helpers                                                            */
/* -------------------------------------------------------------------------- */

static void buttons_init(void) {
  for (uint8_t i = 0; i < NUM_BTNS; i++) {
    pinMode(btn_pin[i], INPUT_PULLUP);
    btn_state[i] = HIGH;
    btn_last[i] = HIGH;
    btn_last_change[i] = millis();
  }
}

static void buttons_update(uint32_t now) {
  for (uint8_t i = 0; i < NUM_BTNS; i++) {
    uint8_t reading = digitalRead(btn_pin[i]);
    if (reading != btn_last[i]) {
      btn_last_change[i] = now;
      btn_last[i] = reading;
    }
    if ((now - btn_last_change[i]) >= DEBOUNCE_MS) {
      if (reading != btn_state[i]) {
        btn_state[i] = reading;
      }
    }
  }
}

/* -------------------------------------------------------------------------- */
/*  Drain encoder edge counters -> decode quadrature                           */
/* -------------------------------------------------------------------------- */

static void encoders_drain(void) {
  // Snapshot the volatile counters; each represents one edge seen by ISR.
  uint8_t n1 = enc1_edges;
  uint8_t n2 = enc2_edges;
  enc1_edges = 0;
  enc2_edges = 0;

  while (n1--) {
    uint8_t now = ((digitalRead(ENC1_CLK) ? 1 : 0) << 0)
                | ((digitalRead(ENC1_DT)  ? 1 : 0) << 1);
    encoder1_count += qtrans[enc1_state][now];
    enc1_state = now;
  }
  while (n2--) {
    uint8_t now = ((digitalRead(ENC2_CLK) ? 1 : 0) << 0)
                | ((digitalRead(ENC2_DT)  ? 1 : 0) << 1);
    encoder2_count += qtrans[enc2_state][now];
    enc2_state = now;
  }
}

/* -------------------------------------------------------------------------- */
/*  OLED render                                                               */
/* -------------------------------------------------------------------------- */

static const char* pressed_str(uint8_t s) {
  // active LOW: pressed when LOW
  return (s == LOW) ? "PRESSED" : "-----  ";
}

static void render_ui(void) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);

  oled.setCursor(0, 0);
  oled.print(F("ENC1: "));
  oled.print(encoder1_count);
  oled.setCursor(64, 0);
  oled.print(F("ENC2: "));
  oled.print(encoder2_count);

  oled.drawLine(0, 9, 127, 9, SSD1306_WHITE);

  oled.setCursor(0, 12);
  oled.print(F("ENC1 SW: "));
  oled.print(pressed_str(btn_state[4]));

  oled.setCursor(0, 22);
  oled.print(F("ENC2 SW: "));
  oled.print(pressed_str(btn_state[5]));

  oled.drawLine(0, 31, 127, 31, SSD1306_WHITE);

  oled.setCursor(0, 34);
  oled.print(F("SW1: ")); oled.print(pressed_str(btn_state[0]));
  oled.setCursor(64, 34);
  oled.print(F("SW2: ")); oled.print(pressed_str(btn_state[1]));

  oled.setCursor(0, 46);
  oled.print(F("SW3: ")); oled.print(pressed_str(btn_state[2]));
  oled.setCursor(64, 46);
  oled.print(F("SW4: ")); oled.print(pressed_str(btn_state[3]));

  oled.display();
}

/* -------------------------------------------------------------------------- */
/*  Arduino setup / loop                                                      */
/* -------------------------------------------------------------------------- */

void setup() {
  // On the Arduino Micro the hardware I2C pins ARE D2 (SDA) and D3 (SCL);
  // Wire.begin() with no arguments selects the board's default SDA/SCL
  // (which here are exactly D2 and D3).  If you adapt this for another
  // board where the default I2C pins differ, call Wire.begin(SDA, SCL)
  // explicitly instead.
  Wire.begin();
  Wire.setClock(400000UL);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    // OLED not found: blink the RX LED so the user notices.
    pinMode(LED_BUILTIN, OUTPUT);
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH); delay(150);
      digitalWrite(LED_BUILTIN, LOW);  delay(150);
    }
  }

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.print(F("Panel init..."));
  oled.display();

  pcint_init();
  buttons_init();
}

void loop() {
  uint32_t now = millis();

  encoders_drain();
  buttons_update(now);

  // Throttle display to ~30 Hz to avoid flicker / I2C traffic.
  static uint32_t last_ui = 0;
  if ((now - last_ui) >= 33) {
    last_ui = now;
    render_ui();
  }
}