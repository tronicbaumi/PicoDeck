#include <SPI.h>
#include <Wire.h>
#include <Keyboard.h>   // USB HID keyboard (ATmega32u4: Micro / Leonardo)
#include <Adafruit_SleepyDog.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#define OLED32 1
#define OLED64_09 1
//#define OLED64_13 1


#ifdef OLED32
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 32 // OLED display height, in pixels
  #define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#elif OLED64_09
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 64 // OLED display height, in pixels
  #define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#elif OLED64_13
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 32 // OLED display height, in pixels
  #define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#endif



// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include "logo.h"   // PicoScope 7 startup logo

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };


#define MAIN_TIME   1000   //ms
int new_time, old_time;

#define SLEEP_DELAY 1     // debug
//#define SLEEP_DELAY 3     // debug
//#define SLEEP_DELAY 75    // 10min
//#define SLEEP_DELAY 150   // 20min
//#define SLEEP_DELAY 225   // 30min
//#define SLEEP_DELAY 450   // 60min

// #define LED1  A1
// #define LED2  A2
// #define LED3  A3
// #define SW1   6
// #define SW2   9

// Rotary encoders
#define ENC1_SW   4
#define ENC1_DT   5
#define ENC1_CLK  6
#define ENC2_SW   7
#define ENC2_DT   8
#define ENC2_CLK  9

struct Encoder
{
  uint8_t pinSW, pinDT, pinCLK;
  uint8_t state;      // last 2-bit CLK/DT state
  int8_t  sub;        // quadrature sub-steps accumulated (+-4 = one detent)
  long    position;   // detent count
  bool    pressed;    // SW currently held (active low), debounced
  char    key;        // character sent over USB when SW is pressed
  uint32_t lastChange;// millis() of last raw SW change (debounce)
  bool    rawSW;      // last raw SW reading
  uint8_t keyCW;      // key sent per clockwise detent (uint8_t: special keys are >127)
  uint8_t keyCCW;     // key sent per counter-clockwise detent
};

// Characters sent to the PC as USB keyboard input when the encoder SW is pressed
#define ENC1_KEY  0     // 0 = none; encoder 1 SW toggles amplitude/offset instead
#define ENC2_KEY  0     // 0 = none; encoder 2 SW toggles TIME/TRIG instead
#define SW_DEBOUNCE_MS 25

// Push buttons (external pull-ups, active low) - key is typed on release
#define BTN1_PIN  10
#define BTN2_PIN  14
#define BTN3_PIN  15
#define BTN4_PIN  16
#define BTN1_KEY  ' '
#define BTN2_KEY  '2'
#define BTN3_KEY  0       // 0 = no key; BTN3 cycles Channel instead
#define BTN4_KEY  'X'

#define BTN_CHANNEL_PIN  BTN3_PIN   // button that cycles Channel
#define CHANNEL_MAX      4
uint8_t Channel = 1;                // 1..CHANNEL_MAX, shown as "CH x" on the OLED
// Encoder 2 mode, cycled by the encoder 2 switch:
//   TIME = cursor up/down, TRIG = trigger level (CW 'b', CCW 'v'), PRE = CW 'm', CCW 'n'
enum { E2_TIME = 0, E2_TRIG, E2_PRE, E2_COUNT };
uint8_t Enc2Mode = E2_TIME;

// Encoder 1 mode per channel (index = Channel-1), cycled by the encoder 1 switch:
//   AMP = plain key, OFF = SHIFT + key (directions swapped), FLT = ALT + key
enum { MODE_AMP = 0, MODE_OFF, MODE_FLT, MODE_COUNT };
uint8_t ChannelMode[CHANNEL_MAX] = {MODE_AMP, MODE_AMP, MODE_AMP, MODE_AMP};
#define CUR_MODE  ChannelMode[Channel - 1]

struct Button
{
  uint8_t  pin;
  char     key;
  bool     pressed;     // debounced state
  bool     rawState;    // last raw reading
  uint32_t lastChange;  // millis() of last raw change
};

Button buttons[4] = {
  {BTN1_PIN, BTN1_KEY, false, false, 0},
  {BTN2_PIN, BTN2_KEY, false, false, 0},
  {BTN3_PIN, BTN3_KEY, false, false, 0},
  {BTN4_PIN, BTN4_KEY, false, false, 0},
};

// Characters sent per rotation detent (clockwise / counter-clockwise)
#define ENC1_KEY_CW   '+'
#define ENC1_KEY_CCW  '-'
#define ENC2_KEY_CW   KEY_UP_ARROW
#define ENC2_KEY_CCW  KEY_DOWN_ARROW

Encoder enc1 = {ENC1_SW, ENC1_DT, ENC1_CLK, 0, 0, 0, false, ENC1_KEY, 0, false, ENC1_KEY_CW, ENC1_KEY_CCW};
Encoder enc2 = {ENC2_SW, ENC2_DT, ENC2_CLK, 0, 0, 0, false, ENC2_KEY, 0, false, ENC2_KEY_CW, ENC2_KEY_CCW};

