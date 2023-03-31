#include <Arduino.h>

/***************
 * Simulator Hub for BHS Flight and Racing Simulator
 *
 * Serial port is used to communicate with FlyPT,
 * then the data is sent to the three ODrive
 * controllers via UART.
 **************
 */

#include <Adafruit_NeoPixel.h>

#ifdef ARDUINO_AVR_NANO
#define LED_PIN 3
#define STATUS_PIN 2
#define NUM_PIXELS 6
#endif

#define LED_PIN 3
#define STATUS_PIN 2
#define NUM_PIXELS 6

#define LED_BLINK_INTERVAL 750 // ms

enum FlyPTcommands
{
  FLYPT_CMD = '$',
  FLYPT_END = '!',
  FLYPT_START = 'S',
  FLYPT_STOP = 'E',
  FLYPT_FRAME = 'F',
  FLYPT_AXIS = 'A',
};

enum LedStates
{
  LEDS_OFF = 0,
  LEDS_ON = 1,
  LEDS_BLINK = 2,
  LEDS_DATA = 3,
};

class MotorValues
{
public:
  uint16_t rawBytes[6] = {0, 0, 0, 0, 0, 0}; // 0-65535
  float position[6] = {0, 0, 0, 0, 0, 0};    // -40 to 40
  int8_t speed[6] = {0, 0, 0, 0, 0, 0};      // -127 to 127
};

// Use neopixels to represent motor values, for now
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

MotorValues motors;
LedStates ledState = LEDS_OFF;
unsigned long lastLedUpdate = 0;
bool blinkState = false;

bool gotCmd = false;

void processIncomingData();
void parseMotorValues();
void updateMotors();
void updateLeds();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(38400);
  pixels.begin();

  pinMode(STATUS_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, LOW);
}

void loop()
{
  // put your main code here, to run repeatedly:
  processIncomingData();
  updateLeds();
}

void processIncomingData()
{
  /**
   * All commands are prefixed with a '$' character
   *
   * Frames are sent beginning with a 'F' character, followed by
   * an integer representing the motor number. The next 2 bytes
   * are the position of the motor (unsigned 16-bit integer, little
   * endian). The next byte is a signed integer of the speed value.
   * The frame is closed with a '!' character.
   *
   * Example frame:
   *  $ F 1 \x01 \x23 \x03 2 \x04 \x56 \x07 3 \x08 \x79 \x0B 4 \x0C \x9A \x0D 5 \x0E \xBC \x0F 6 \x10 \xDE \x11 !
   *
   */
  // Read the incoming data
  while (Serial.available() > 0)
  {
    char c = Serial.read();
    switch (c)
    {
    case FLYPT_CMD:
      // Next character is the command
      digitalWrite(STATUS_PIN, LOW);
      gotCmd = true;
      break;
    case FLYPT_START:
      // Start the motors
      if (!gotCmd)
        break;

      for (int i = 0; i < 6; i++)
      {
        motors.rawBytes[i] = 0;
      }
      ledState = LEDS_BLINK;

      gotCmd = false;
      break;

    case FLYPT_STOP:
      // Stop the motors
      if (!gotCmd)
        break;

      ledState = LEDS_OFF;

      gotCmd = false;
      break;

    case FLYPT_FRAME:
      // Read the next x bytes and parse them as motor values
      if (!gotCmd)
        break;

      ledState = LEDS_DATA;
      parseMotorValues();

      gotCmd = false;
      break;

    default:
      // Ignore the character
      break;
    }
  }
}

void waitForBuffer(int size)
{
  while (Serial.available() < size)
  {
    // Wait for the next x bytes
  }
}

void parseMotorValues()
{
  while (true)
  {
    waitForBuffer(1); // 1 byte for motor number

    char c = Serial.read();
    if (c == FLYPT_END)
    {
      // We're done reading the motor values
      updateMotors();
      break;
    }

    waitForBuffer(3); // 2 bytes for motor value, 1 byte for speed

    // The next character is the motor number (char starts at 1, motor starts at 0);
    uint8_t motorNum = c - '1';

    // The next 2 bytes are the motor value
    byte msb = Serial.read();
    byte lsb = Serial.read();

    int8_t speed = Serial.read();

    // Combine the bytes into a single value
    int motorValue = (msb << 8) | lsb;

    motors.rawBytes[motorNum] = motorValue;
    motors.position[motorNum] = map(motorValue, 0, 65535, -40, 40);
    motors.speed[motorNum] = speed;
  }
}

void updateLeds()
{
  switch (ledState)
  {
  case LEDS_OFF:
    pixels.clear();
    break;
  case LEDS_ON:
    for (int i = 0; i < NUM_PIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(100, 0, 0));
    }
    break;
  case LEDS_BLINK:
  {
    if (blinkState)
    {
      for (int i = 0; i < NUM_PIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color(20, 80, 0));
      }
    }
    else
    {
      pixels.clear();
    }

    if ((millis() - lastLedUpdate) >= LED_BLINK_INTERVAL)
    {
      blinkState = !blinkState;
      lastLedUpdate = millis();
    }
    break;
  }

  case LEDS_DATA:
    // for (int i = 0; i < NUM_PIXELS; i++)
    // {
    //   int color = map(motors.rawBytes[i], 0, 65535, 0, 100);
    //   pixels.setPixelColor(i, pixels.Color(0, color, 0));
    // }
    break;
  }

  pixels.show();
}

void updateMotors()
{
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    int color = map(motors.rawBytes[i], 0, 65535, 0, 255);
    pixels.setPixelColor(i, pixels.Color(0, color, 0));
  }

  pixels.show();
}