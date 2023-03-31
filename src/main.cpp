#include <Arduino.h>

/***************
 * Simulator Hub for BHS Flight and Racing Simulator
 *
 * Serial port is used to communicate with FlyPT,
 * then the data is sent to the three ODrive
 * controllers via UART.
 *
 * Input ranges: 0-65535 (16-bit unsigned integer)
 * Motor output: -40 to 40 (float)
 **************
 */

#include <Adafruit_NeoPixel.h>

#define LED_PIN 6
// #define LED_PIN 3
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

enum HubStates
{
  BOOT = 0,
  IDLE = 1,
  RUNNING = 2,
};

class MotorValues
{
public:
  uint16_t rawBytes[6] = {0, 0, 0, 0, 0, 0}; // 0-65535
  float position[6] = {0, 0, 0, 0, 0, 0};    // -40 to 40
};

// Use neopixels to represent motor values, for now
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
MotorValues motors;
HubStates hubState = BOOT;

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
  Serial.begin(115200);
  pixels.begin();
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
   * endian). The frame is closed with a '!' character.
   *
   * Example frame:
   *  $ F 1 \x01 \x23 2 \x04 \x56 3 \x08 \x79 4 \x0C \x9A 5 \x0E \xBC 6 \x10 \xDE !
   *
   * Each frame is 21 bytes long.
   */
  // Read the incoming data
  while (Serial.available() > 0)
  {
    char c = Serial.read();
    switch (c)
    {
    case FLYPT_CMD:
      // Next character is the command
      gotCmd = true;
      break;
    case FLYPT_START:
      // Start the motors
      if (!gotCmd)
        break;

      hubState = IDLE;

      gotCmd = false;
      break;

    case FLYPT_STOP:
      // Stop the motors
      if (!gotCmd)
        break;

      hubState = IDLE;

      gotCmd = false;
      break;

    case FLYPT_FRAME:
      // Read the next x bytes and parse them as motor values
      if (!gotCmd)
        break;

      hubState = RUNNING;
      parseMotorValues();

      gotCmd = false;
      break;

    default:
      // Ignore the character
      gotCmd = false;
      break;
    }
  }
}

void waitForBuffer(int size)
{
  while (Serial.available() < size)
    ;
}

void parseMotorValues()
{
  uint8_t motorNum = 0;

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

    waitForBuffer(2); // Motor value is 2 bytes

    // The next character is the motor number (char starts at 1, motor starts at 0);
    motorNum = c - '1';

    // The next 2 bytes are the motor value
    byte msb = Serial.read();
    byte lsb = Serial.read();

    // Combine the bytes into a single value
    int motorValue = (msb << 8) | lsb;

    motors.rawBytes[motorNum] = motorValue;

    // Convert the motor value to a float between -40 and 40
    motors.position[motorNum] = ((80.0 / 65535.0) * float(motorValue)) - 40.0;
  }
}

void updateLeds()
{
  switch (hubState)
  {
  case BOOT:
    pixels.clear();
    break;
  case RUNNING:
    for (int i = 0; i < 6; i++)
    {
      pixels.setPixelColor(i, pixels.Color((blinkState) ? 100 : 0, 0, 0));
      blinkState = !blinkState;
    }
    break;
  case IDLE:
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
  }

  pixels.show();
}

void updateMotors()
{
  // TODO
}