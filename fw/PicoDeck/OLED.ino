// Layout (text size 2 = 12x16 px per character, 128x64 display):
//   y=0   header "PicoDECK" (centered)
//   y=24  "CH x" (left)
//   y=48  channel mode "AMP/OFF/FLT" (left), encoder 2 mode "TIME/TRIG/PRE" (right)
#define ROW_HEADER  0
#define ROW_CHANNEL 24
#define ROW_ENC2    48
#define CHAR_W      12
#define CHAR_H      16

void Draw_Header(void)
{
  display.fillRect(0, ROW_HEADER, SCREEN_WIDTH, CHAR_H, SSD1306_BLACK);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor((SCREEN_WIDTH - 8 * CHAR_W) / 2, ROW_HEADER);
  display.print(F("PicoDECK"));
  display.display();
}

// Shows the channel mode "AMP" / "OFF" / "FLT" at the bottom left
static void Draw_ChannelMode(void)
{
  display.fillRect(0, ROW_ENC2, SCREEN_WIDTH / 2, CHAR_H, SSD1306_BLACK);   // left half only
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, ROW_ENC2);
  switch (CUR_MODE)
  {
    case MODE_OFF: display.print(F("OFF")); break;
    case MODE_FLT: display.print(F("FLT")); break;
    default:       display.print(F("AMP")); break;
  }
}

// Shows "CH x" on the channel row and the channel's mode at the bottom left
void Draw_Channel(void)
{
  display.fillRect(0, ROW_CHANNEL, SCREEN_WIDTH, CHAR_H, SSD1306_BLACK);   // clear old text only
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, ROW_CHANNEL);
  display.print(F("CH "));
  display.print(Channel);
  Draw_ChannelMode();
  display.display();
}

// Shows "TIME" / "TRIG" / "PRE" (encoder 2 mode), right aligned
void Draw_Enc2Mode(void)
{
  const __FlashStringHelper *txt;
  uint8_t chars;
  switch (Enc2Mode)
  {
    case E2_TRIG: txt = F("TRIG"); chars = 4; break;
    case E2_PRE:  txt = F("PRE");  chars = 3; break;
    default:      txt = F("TIME"); chars = 4; break;
  }
  display.fillRect(SCREEN_WIDTH / 2, ROW_ENC2, SCREEN_WIDTH / 2, CHAR_H, SSD1306_BLACK);   // right half only
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH - chars * CHAR_W, ROW_ENC2);
  display.print(txt);
  display.display();
}

void Init_Display(void)
{
  Serial.print("Initializing Display..."); 
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
    Serial.println(F("SSD1306 allocation failed"));
  //  for(;;); // Don't proceed, loop forever
  }
  Serial.println("Done!");
  Serial.println(display.height());
  // The library preloads its buffer with the Adafruit splash; replace it with
  // the PicoScope 7 logo (centered) before the first display() call.
  display.clearDisplay();
  display.drawBitmap((SCREEN_WIDTH - PS7_LOGO_WIDTH) / 2, (SCREEN_HEIGHT - PS7_LOGO_HEIGHT) / 2,
                     ps7_logo_bmp, PS7_LOGO_WIDTH, PS7_LOGO_HEIGHT, SSD1306_WHITE);
  display.display();
  delay(2000); // Pause for 2 seconds

  // while(1)
  // {
  //   display.clearDisplay();
  //   display.setTextSize(1);             // Normal 1:1 pixel scale
  //   display.setTextColor(SSD1306_WHITE);        // Draw white text
  //   display.setCursor(0,0);             // Start at top-left corner
  //   display.println(F("Initializing..."));
  //   display.display();
  //   delay(2000);
  // }
}