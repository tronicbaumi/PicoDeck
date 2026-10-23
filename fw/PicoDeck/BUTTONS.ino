
void Init_Buttons(void)
{
  Serial.print("Initializing buttons...");
  for (uint8_t i = 0; i < 4; i++)
  {
    Button &b = buttons[i];
    pinMode(b.pin, INPUT);          // external pull-ups fitted
    b.rawState = b.pressed = !digitalRead(b.pin);
    b.lastChange = millis();
  }
  Serial.println("done!");
}

// Poll every loop pass; types b.key on the PC when a button is released.
void Update_Buttons(void)
{
  uint32_t now = millis();
  for (uint8_t i = 0; i < 4; i++)
  {
    Button &b = buttons[i];
    bool raw = !digitalRead(b.pin);   // active low
    if (raw != b.rawState) { b.rawState = raw; b.lastChange = now; }
    if (raw != b.pressed && (now - b.lastChange) >= SW_DEBOUNCE_MS)
    {
      b.pressed = raw;
      if (!b.pressed)
      {
        if (b.pin == BTN_CHANNEL_PIN)
        {
          Channel = (Channel % CHANNEL_MAX) + 1;   // 1..4, then back to 1
          Draw_Channel();
        }
        if (b.key) Keyboard.write(b.key);
        Serial.print("BTN"); Serial.print(i + 1); Serial.println(" released");
      }
    }
  }
}
