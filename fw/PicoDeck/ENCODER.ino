
static void Init_Encoder(Encoder &e)
{
  pinMode(e.pinSW, INPUT_PULLUP);
  pinMode(e.pinDT, INPUT_PULLUP);
  pinMode(e.pinCLK, INPUT_PULLUP);
  e.state = (digitalRead(e.pinCLK) << 1) | digitalRead(e.pinDT);
  e.sub = 0;
  e.position = 0;
  e.rawSW = e.pressed = !digitalRead(e.pinSW);
  e.lastChange = millis();
}

void Init_Encoders(void)
{
  Serial.print("Initializing encoders...");
  Init_Encoder(enc1);
  Init_Encoder(enc2);
  Keyboard.begin();
  Serial.println("done!");
}

// Encoder 1 keys per channel (index = Channel-1); 0 = use plain key.
static const char enc1CtrlCW[CHANNEL_MAX]  = {'q', 'w', 'e', 'r'};
static const char enc1CtrlCCW[CHANNEL_MAX] = {'1', '2', '3', '4'};

// Hold a modifier around a key press, long enough for programs that poll it.
static void Send_Combo(uint8_t mod, char k)
{
  Keyboard.press(mod);
  delay(30);
  Keyboard.press(k);
  delay(30);
  Keyboard.release(k);
  delay(30);
  Keyboard.release(mod);
}

// ALT + key sent when switching INTO and OUT OF filter mode (index = Channel-1)
static const char enc1FltEnter[CHANNEL_MAX] = {'a', 's', 'd', 'f'};

// Send the key for one rotation detent. Encoder 1 types the keys above on
// each channel; if an entry is 0 it falls back to the encoder's plain key.
static void Send_Rotation(Encoder &e, bool cw)
{
  if (&e == &enc1)
  {
    // offset mode swaps the directions: cw sends the ccw key and vice versa
    uint8_t mode = CUR_MODE;
    bool useCW = (mode == MODE_OFF) ? !cw : cw;
    char k = useCW ? enc1CtrlCW[Channel - 1] : enc1CtrlCCW[Channel - 1];
    if (k)
    {
      if (mode == MODE_AMP)
        Keyboard.write(k);                // amplitude = plain key only
      else
      {
        // offset = key + SHIFT, filter = key + ALT. The modifier is held as a real
        // modifier for the whole keystroke, long enough for programs that poll it.
        Send_Combo((mode == MODE_OFF) ? KEY_LEFT_SHIFT : KEY_LEFT_ALT, k);
      }
      Serial.print("ENC1 CH"); Serial.print(Channel);
      Serial.println(mode == MODE_OFF ? " OFF" : mode == MODE_FLT ? " FLT" : " AMP");
      return;
    }
  }
  if (&e == &enc2 && Enc2Mode == E2_TRIG)
  {
    Keyboard.write(cw ? 'b' : 'v');   // trigger level: CW = B, CCW = V
    return;
  }
  if (&e == &enc2 && Enc2Mode == E2_PRE)
  {
    Keyboard.write(cw ? 'm' : 'n');   // PRE: CW = M, CCW = N
    return;
  }
  Keyboard.write(cw ? e.keyCW : e.keyCCW);   // TIME mode: unchanged
}

// Poll one encoder; call as often as possible (every loop pass).
// Returns +1 / -1 on a full detent step, 0 otherwise.
static int8_t Update_Encoder(Encoder &e)
{
  // transition table indexed by (oldState<<2 | newState)
  static const int8_t table[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

  // debounced SW; types e.key on the PC on each press (not on release)
  bool raw = !digitalRead(e.pinSW);
  uint32_t now = millis();
  if (raw != e.rawSW) { e.rawSW = raw; e.lastChange = now; }
  if (raw != e.pressed && (now - e.lastChange) >= SW_DEBOUNCE_MS)
  {
    e.pressed = raw;
    if (e.pressed)
    {
      if (&e == &enc1)
      {
        // leaving FLT: same ALT combo as when entering it
        if (CUR_MODE == MODE_FLT) Send_Combo(KEY_LEFT_ALT, enc1FltEnter[Channel - 1]);
        CUR_MODE = (CUR_MODE + 1) % MODE_COUNT;   // AMP -> OFF -> FLT -> AMP, current channel only
        // entering FLT
        if (CUR_MODE == MODE_FLT) Send_Combo(KEY_LEFT_ALT, enc1FltEnter[Channel - 1]);
        Draw_Channel();
        Serial.print("CH"); Serial.print(Channel);
        Serial.println(CUR_MODE == MODE_OFF ? " -> OFFSET" : CUR_MODE == MODE_FLT ? " -> FILTER" : " -> AMPLITUDE");
      }
      if (&e == &enc2)
      {
        Enc2Mode = (Enc2Mode + 1) % E2_COUNT;   // TIME -> TRIG -> PRE -> TIME
        Draw_Enc2Mode();
        Serial.println(Enc2Mode == E2_TRIG ? "ENC2 -> TRIG" : Enc2Mode == E2_PRE ? "ENC2 -> PRE" : "ENC2 -> TIME");
      }
      if (e.key) Keyboard.write(e.key);
    }
  }

  uint8_t s = (digitalRead(e.pinCLK) << 1) | digitalRead(e.pinDT);
  if (s == e.state) return 0;

  e.sub += table[(e.state << 2) | s];
  e.state = s;

  if (e.sub >= 4)  { e.sub = 0; e.position++; Send_Rotation(e, true);  return 1; }
  if (e.sub <= -4) { e.sub = 0; e.position--; Send_Rotation(e, false); return -1; }
  return 0;
}

void Update_Encoders(void)
{
  int8_t d1 = Update_Encoder(enc1);
  int8_t d2 = Update_Encoder(enc2);

  if (d1) { Serial.print("ENC1 pos=");  Serial.println(enc1.position); }
  if (d2) { Serial.print("ENC2 pos=");  Serial.println(enc2.position); }
}
