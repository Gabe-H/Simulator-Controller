#include <Arduino.h>

/***************
 * Simulator Hub for BHS Flight and Racing Simulator
 *
 * Serial port is used to communicate with FlyPT,
 * then the data is sent to the three ODrive
 * controllers via UART.
 *
 * Input ranges: 0-65535 (16-bit unsigned integer)
 * Motor output: -34 to 34 (float)
 **************
 */

#include <SimulatorHub.h>
#include <Button.h>
#include <StatusPin.h>
#include "config.h"

// #define DEBUG(x) (odrv0.println(x))
#define DEBUG(x)

SimulatorHub hub(odrv0, odrv1, odrv2);
Button startButton(START_BUTTON);
Button stopButton(STOP_BUTTON);
StatusPin led(STATUS_PIN); // Led for now, will be bigger light on control panel later

// State change handler fnct. declaration
void handleState(HubStates state);

void setup()
{
  // Setup ODrive serial ports
  Serial.begin(BAUD_RATE);
  odrv0.begin(BAUD_RATE);
  odrv1.begin(BAUD_RATE);
  odrv2.begin(BAUD_RATE);

  // Init led pin
  led.setup();

  // Initialize hub (doesn't do much now, but there if needed)
  hub.setup();
}

void loop()
{
  // Process incoming data and send to ODrives
  if (hub.processIncomingData())
  { /* Data was received from FlyPT */
  }

  // Handle button presses
  if (stopButton.handle())
    hub.stopSimulator(); // Stop data stream, move position down for rider to get off

  else if (startButton.handle())
    hub.startSimulator(); // Reset position to 0, then start data stream

  // Handle state changes
  if (hub.stateChange())
    handleState(hub.getState());

  // Handle led/light
  led.loop();

  // Breathing room (bogging at 500us, okay at 750us. Playing safe with 1ms)
  delayMicroseconds(1000);
}

void handleState(HubStates state)
{
  // These are only called once, when the state changes
  switch (state)
  {
  case STARTING:
    // Set fast blink rate (don't see this much)
    led.setDelay(100);
    led.setMode(LED_BLINK);
    DEBUG("Starting...");
    break;

  case RUNNING:
    // Led off during operation
    led.setMode(LED_OFF);
    DEBUG("Running...");
    break;

  case STOPPED:
    // Set slow blink rate
    led.setDelay(1000);
    led.setMode(LED_BLINK);
    DEBUG("FlyPT stopped");
    break;

  case READY:
    // Medium blink rate for ready
    led.setDelay(350);
    led.setMode(LED_BLINK);
    DEBUG("Ready for FlyPT");
    break;

  case IDLE:
    // Led solid on when idle
    led.setMode(LED_ON);
    DEBUG("Idle");
    break;
  }
}
