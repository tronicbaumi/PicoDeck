
#include "general.h"


#define UART

#define SAVE_POWER
#define SCHEDULER

//char value0char[8] = "";
//char value1char[8] = "";
char value2char[8] = "";
char value3char[8] = "";
//char value4char[8] = "";
//char value5char[8] = "";

//String value0;
//String value1;
String value2;
String value3;
//String value4;
//String value5;



unsigned int count1=0;

void setup() 
{
 // delay(5000);
  Serial.begin(115200);
  Serial.println("\n\n\n\n\n");
  Serial.println("PicoDeck started....");
  Serial.println("===========================");

  Init_Display();
  display.clearDisplay();
  display.setTextSize(2);             // 2x pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  //display.println(F("Init..."));
  display.display();
  //delay(2000);

  Init_Encoders();
  Init_Buttons();

  display.clearDisplay();
  Draw_Header();
  Draw_Channel();
  Draw_Enc2Mode();

  old_time = millis();
  Serial.println("Main Loop");
}



void loop() 
{
  //Serial.print(new_time);Serial.print(";");Serial.println(old_time);
  // encoders must be polled every pass, not only once per MAIN_TIME
  Update_Encoders();
  Update_Buttons();

  new_time = millis();
  if ((new_time-old_time) >= MAIN_TIME)
  {
    old_time = new_time;

    // example: show encoder positions and button state once per second
    Serial.print("ENC1: ");  Serial.print(enc1.position);
    Serial.print(enc1.pressed ? " [SW]" : "     ");
    Serial.print("  ENC2: "); Serial.print(enc2.position);
    Serial.println(enc2.pressed ? " [SW]" : "");
  }
}

