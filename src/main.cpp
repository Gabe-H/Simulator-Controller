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
#include <SimulatorHub.h>

#define LED_PIN 6
// #define LED_PIN 3
#define NUM_PIXELS 6

#define LED_BLINK_INTERVAL 750 // ms

// Use neopixels to represent motor values, for now
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
HubStates hubState;
SimulatorHub hub;

unsigned long lastLedUpdate = 0;
bool blinkState = false;

bool gotCmd = false;

void updateLeds();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pixels.begin();

  hub.setup();
}

void loop()
{
  // put your main code here, to run repeatedly:
  hubState = hub.loop();
  updateLeds();
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