/*
  KeyboardAndMouseControl

  Controls the mouse from five pushbuttons on an Arduino Leonardo, Micro or Due.

  Hardware:
  - five pushbuttons attached to D2, D3, D4, D5, D6

  The mouse movement is always relative. This sketch reads four pushbuttons, and
  uses them to set the movement of the mouse.

  WARNING: When you use the Mouse.move() command, the Arduino takes over your
  mouse! Make sure you have control before you use the mouse commands.

  created 15 Mar 2012
  modified 27 Mar 2012
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/KeyboardAndMouseControl
*/

#include "Keyboard.h"
//#include "Mouse.h"

// set pin numbers for the five buttons:
const int btn5 = 2;
const int btn4 = 3;
const int btn3 = 4;
const int btn2 = 5;
const int btn1 = 6;

char btn1_old=65,btn2_old=65,btn3_old=65,btn4_old=65,btn5_old=65;
char btn1_new, btn2_new, btn3_new, btn4_new, btn5_new;

void setup() { // initialize the buttons' inputs:
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(btn3, INPUT);
  pinMode(btn4, INPUT);
  pinMode(btn5, INPUT);

  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB
//  }
  Serial.println(" MS Teams Keyboard started......");

  Keyboard.begin();
}

void loop() {
 
  // use the pushbuttons to control the keyboard:
  BTN1();
  BTN2();
  BTN3();
  BTN4();
  BTN5();
  delay(10);
}


void BTN1(void)       // TOGGLE AUDIO
{
  if (digitalRead(btn1) == HIGH) btn1_new = 65;
  else btn1_new = 66;

  if (btn1_new != btn1_old)
  {
    if(btn1_new == 65)
    {
      Serial.println("BTN1 released");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press('M');
      delay(50);
      Keyboard.releaseAll();
    }
    if(btn1_new == 66)
    {
      Serial.println("BTN1 pressed");  
    }
    btn1_old = btn1_new;
  }
}

void BTN2(void)       // TOGGLE VIDEO
{
  if (digitalRead(btn2) == HIGH) btn2_new = 65;
  else btn2_new = 66;

  if (btn2_new != btn2_old)
  {
    if(btn2_new == 65)
    {
      Serial.println("BTN2 released");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press('O');
      delay(50);
      Keyboard.releaseAll();
    }
    if(btn2_new == 66)
    {
      Serial.println("BTN2 pressed");  
    }
    btn2_old = btn2_new;
  }
}


void BTN3(void)       // TOGGLE BLUR
{
  if (digitalRead(btn3) == HIGH) btn3_new = 65;
  else btn3_new = 66;

  if (btn3_new != btn3_old)
  {
    if(btn3_new == 65)
    {
      Serial.println("BTN3 released");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press('P');
      delay(50);
      Keyboard.releaseAll();
    }
    if(btn3_new == 66)
    {
      Serial.println("BTN3 pressed");  
    }
    btn3_old = btn3_new;
  }
}


void BTN4(void)       // TOGGLE HAND
{
  if (digitalRead(btn4) == HIGH) btn4_new = 65;
  else btn4_new = 66;

  if (btn4_new != btn4_old)
  {
    if(btn4_new == 65)
    {
      Serial.println("BTN4 released");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press('K');
      delay(50);
      Keyboard.releaseAll();
    }
    if(btn4_new == 66)
    {
      Serial.println("BTN4 pressed");  
    }
    btn4_old = btn4_new;
  }
}

void BTN5(void)       // SHARE SCREEN
{
  if (digitalRead(btn5) == HIGH) btn5_new = 65;
  else btn5_new = 66;

  if (btn5_new != btn5_old)
  {
    if(btn5_new == 65)
    {
      Serial.println("BTN5 released");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_SHIFT);
      Keyboard.press('E');
      delay(50);
      Keyboard.releaseAll();
    }
    if(btn5_new == 66)
    {
      Serial.println("BTN5 pressed");  
    }
    btn5_old = btn5_new;
  }
}
