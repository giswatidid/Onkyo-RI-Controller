#include <IRremote.hpp>

const byte RI_PIN = 8;
const byte IR_PIN = 2;


// =========================================================
// PIONEER REMOTE CODES
// =========================================================

const uint32_t PIONEER_CD    = 0xB34C5AA5;
const uint32_t PIONEER_AUX   = 0xFB045AA5;

const uint32_t PIONEER_PLAY  = 0x3FC05AA5;
const uint32_t PIONEER_PAUSE = 0x3EC15AA5;
const uint32_t PIONEER_FF    = 0xD12E50AF;
const uint32_t PIONEER_REW   = 0xD22D50AF;

const uint32_t PIONEER_BACK  = 0xF6095AA5;


// =========================================================
// ONKYO RI COMMANDS
// =========================================================

// C-705TX
const uint16_t CD_ON    = 0xF04;
const uint16_t CD_PLAY  = 0xF1B;
const uint16_t CD_PAUSE = 0xF1F;
const uint16_t CD_NEXT  = 0xF1D;
const uint16_t CD_PREV  = 0xF1E;

// K-505X
const uint16_t TAPE_STOP         = 0xD13;
const uint16_t TAPE_FORWARD_PLAY = 0xD15;
const uint16_t TAPE_REVERSE_PLAY = 0xD16;
const uint16_t TAPE_FF           = 0xD19;
const uint16_t TAPE_REW          = 0xD1A;


// =========================================================
// CONTROL MODE
// =========================================================

enum ControlMode
{
  MODE_UNKNOWN,
  MODE_CD,
  MODE_TAPE
};

ControlMode currentMode = MODE_UNKNOWN;


// =========================================================
// TAPE DIRECTION
// =========================================================

enum TapeDirection
{
  TAPE_FORWARD,
  TAPE_REVERSE
};

TapeDirection tapeDirection = TAPE_FORWARD;

bool tapePlaying = false;


// =========================================================
// DUPLICATE FILTER
// =========================================================

uint32_t lastAcceptedCode = 0;
unsigned long lastAcceptedTime = 0;

const unsigned long DEBOUNCE_MS = 300;

bool isDuplicate(uint32_t code)
{
  unsigned long now = millis();

  if (code == lastAcceptedCode &&
      (now - lastAcceptedTime) < DEBOUNCE_MS)
  {
    return true;
  }

  lastAcceptedCode = code;
  lastAcceptedTime = now;

  return false;
}


// =========================================================
// ONKYO RI TRANSMITTER
// =========================================================

void sendRI(uint16_t command)
{
  noInterrupts();

  digitalWrite(RI_PIN, LOW);

  // Header
  digitalWrite(RI_PIN, HIGH);
  delayMicroseconds(3000);

  digitalWrite(RI_PIN, LOW);
  delayMicroseconds(1000);

  // 12 bits, MSB first
  for (int bit = 11; bit >= 0; bit--)
  {
    digitalWrite(RI_PIN, HIGH);
    delayMicroseconds(1000);

    digitalWrite(RI_PIN, LOW);

    if (command & (1 << bit))
      delayMicroseconds(2000);
    else
      delayMicroseconds(1000);
  }

  // Trailer
  digitalWrite(RI_PIN, HIGH);
  delayMicroseconds(1000);

  digitalWrite(RI_PIN, LOW);

  interrupts();
}


void sendOnkyo(uint16_t command)
{
  IrReceiver.stop();

  sendRI(command);

  IrReceiver.start();
}


// =========================================================
// PLAY TAPE IN STORED DIRECTION
// =========================================================

void playTape()
{
  if (tapeDirection == TAPE_FORWARD)
  {
    Serial.println("TAPE PLAY -> FORWARD 0xD15");
    sendOnkyo(TAPE_FORWARD_PLAY);
  }
  else
  {
    Serial.println("TAPE PLAY -> REVERSE 0xD16");
    sendOnkyo(TAPE_REVERSE_PLAY);
  }

  tapePlaying = true;
}


// =========================================================
// SETUP
// =========================================================

void setup()
{
  pinMode(RI_PIN, OUTPUT);
  digitalWrite(RI_PIN, LOW);

  Serial.begin(115200);

  IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);

  Serial.println();
  Serial.println("PIONEER -> ONKYO RI CONTROLLER");
  Serial.println("==============================");
  Serial.println();

  Serial.println("CD:");
  Serial.println(" CD    = select / power on");
  Serial.println(" PLAY  = play");
  Serial.println(" PAUSE = pause");
  Serial.println(" FF    = next track");
  Serial.println(" REW   = previous track");
  Serial.println();

  Serial.println("TAPE:");
  Serial.println(" AUX   = select / wake");
  Serial.println(" PLAY  = play in selected direction");
  Serial.println(" PAUSE = stop");
  Serial.println(" FF    = fast forward");
  Serial.println(" REW   = rewind");
  Serial.println(" BACK  = forward/reverse");
  Serial.println();

  Serial.println("Initial tape direction = FORWARD");
  Serial.println("Ready.");
}


// =========================================================
// MAIN LOOP
// =========================================================

void loop()
{
  if (!IrReceiver.decode())
    return;

  uint32_t code = IrReceiver.decodedIRData.decodedRawData;

  Serial.print("IR: 0x");
  Serial.println(code, HEX);


  // =======================================================
  // CD BUTTON
  // Select CD mode + turn C-705TX on
  // =======================================================

  if (code == PIONEER_CD)
  {
    if (!isDuplicate(code))
    {
      currentMode = MODE_CD;

      Serial.println("*** CONTROL MODE = CD ***");
      Serial.println("CD POWER ON -> 0xF04");

      sendOnkyo(CD_ON);
    }
  }


  // =======================================================
  // AUX BUTTON
  // Select Tape mode + wake K-505X
  // =======================================================

  else if (code == PIONEER_AUX)
  {
    if (!isDuplicate(code))
    {
      currentMode = MODE_TAPE;

      Serial.println("*** CONTROL MODE = TAPE ***");
      Serial.println("TAPE WAKE -> 0xD15");

      sendOnkyo(TAPE_FORWARD_PLAY);

      // Waking the deck does not mean we regard it as playing.
      tapePlaying = false;
    }
  }


  // =======================================================
  // BACK
  // Tape mode only:
  // toggle remembered playback direction
  // =======================================================

  else if (code == PIONEER_BACK)
  {
    if (!isDuplicate(code))
    {
      if (currentMode == MODE_TAPE)
      {
        if (tapeDirection == TAPE_FORWARD)
        {
          tapeDirection = TAPE_REVERSE;
          Serial.println("*** TAPE DIRECTION = REVERSE ***");
        }
        else
        {
          tapeDirection = TAPE_FORWARD;
          Serial.println("*** TAPE DIRECTION = FORWARD ***");
        }

        // If already playing, change direction immediately.
        if (tapePlaying)
          playTape();
      }
    }
  }


  // =======================================================
  // PLAY
  // =======================================================

  else if (code == PIONEER_PLAY)
  {
    if (!isDuplicate(code))
    {
      if (currentMode == MODE_CD)
      {
        Serial.println("PLAY -> CD 0xF1B");
        sendOnkyo(CD_PLAY);
      }

      else if (currentMode == MODE_TAPE)
      {
        playTape();
      }
    }
  }


  // =======================================================
  // PAUSE / STOP
  // =======================================================

  else if (code == PIONEER_PAUSE)
  {
    if (!isDuplicate(code))
    {
      if (currentMode == MODE_CD)
      {
        Serial.println("PAUSE -> CD 0xF1F");
        sendOnkyo(CD_PAUSE);
      }

      else if (currentMode == MODE_TAPE)
      {
        Serial.println("STOP -> TAPE 0xD13");
        sendOnkyo(TAPE_STOP);

        tapePlaying = false;
      }
    }
  }


  // =======================================================
  // FF / NEXT TRACK
  // =======================================================

  else if (code == PIONEER_FF)
  {
    if (!isDuplicate(code))
    {
      if (currentMode == MODE_CD)
      {
        Serial.println("NEXT TRACK -> CD 0xF1D");
        sendOnkyo(CD_NEXT);
      }

      else if (currentMode == MODE_TAPE)
      {
        Serial.println("FAST FORWARD -> TAPE 0xD19");
        sendOnkyo(TAPE_FF);

        tapePlaying = false;
      }
    }
  }


  // =======================================================
  // REW / PREVIOUS TRACK
  // =======================================================

  else if (code == PIONEER_REW)
  {
    if (!isDuplicate(code))
    {
      if (currentMode == MODE_CD)
      {
        Serial.println("PREVIOUS TRACK -> CD 0xF1E");
        sendOnkyo(CD_PREV);
      }

      else if (currentMode == MODE_TAPE)
      {
        Serial.println("REWIND -> TAPE 0xD1A");
        sendOnkyo(TAPE_REW);

        tapePlaying = false;
      }
    }
  }


  IrReceiver.resume();
}
