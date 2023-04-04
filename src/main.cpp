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

#define START_BUTTON 11
#define STOP_BUTTON 12

#define odrv0 Serial1
#define odrv1 Serial2
#define odrv2 Serial3

#define BAUD_RATE 115200

SimulatorHub hub(odrv0, odrv1, odrv2);

Button startButton(START_BUTTON);
Button stopButton(STOP_BUTTON);

void handleState(HubStates state);

void setup()
{
  Serial.begin(115200);
  odrv0.begin(115200);

  // Setup ODrive serial ports
  Serial.begin(BAUD_RATE);
  odrv0.begin(BAUD_RATE);
  odrv1.begin(BAUD_RATE);
  odrv2.begin(BAUD_RATE);

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
  if (startButton.handle())
  {
    // Reset position to 0, then start data stream
    hub.startSimulator();
  }
  if (stopButton.handle())
  {
    // Stop data stream, move position down for rider to get off
    hub.stopSimulator();
  }

  // Handle state changes
  if (hub.stateChange())
    handleState(hub.getState());

  delayMicroseconds(500); // Breathing room for serial ports
}

void handleState(HubStates state)
{
  // These are only called once, when the state changes
  switch (state)
  {
  case STARTING:
    odrv0.println("Starting...");
    break;
  case RUNNING:
    odrv0.println("Running...");
    break;
  case STOPPED:
    odrv0.println("FlyPT stopped");
    break;
  case READY:
    odrv0.println("Ready for FlyPT");
    break;
  case IDLE:
    odrv0.println("Idle");
    break;
  }
}
