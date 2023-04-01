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

#include <SimulatorHub.h>

#define START_BUTTON 14
#define STOP_BUTTON 12

class Button
{
public:
  Button(int pin)
  {
    this->pin = pin;
    pinMode(pin, INPUT_PULLUP);
  }

  // Returns true when the button was pressed
  bool handle()
  {
    int state = digitalRead(pin);

    // Button is pressed
    if (state == LOW && oldState == HIGH)
    {
      // Catch bouncing
      if ((millis() - lastDebounceTime) < debounce)
        return false;

      oldState = state;
      lastDebounceTime = millis();
      return true;
    }
    else
    {
      oldState = state;
      return false;
    }
  }

private:
  int pin;
  int oldState = HIGH;
  unsigned long debounce = 100; // ms
  unsigned long lastDebounceTime = 0;
};

Button startButton(START_BUTTON);
Button stopButton(STOP_BUTTON);

HardwareSerial odrv0(1);
SimulatorHub hub(odrv0);

// HardwareSerial odrv0(1);
// HardwareSerial odrv1(2);
// HardwareSerial odrv2(3);
// SimulatorHub hub(odrv0, odrv1, odrv2);

unsigned long loopCount = 0;
unsigned long timer = 0;

void handleState(HubStates state);

void setup()
{
  Serial.begin(115200);

  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(START_BUTTON, INPUT_PULLUP);

  // Setup ODrive serial ports
  hub.setup();
}

void loop()
{
  // Process incoming data and send to ODrives
  HubStates hubState = hub.loop();

  // Handle state changes (not really implemented yet)
  handleState(hubState);

  // Handle button presses
  if (startButton.handle())
  {
    hub.startSimulator();
  }
  else if (stopButton.handle())
  {
    hub.stopSimulator();
  }

  /* Print loop count every second to determine
   * how fast we can run FlyPT
   * [DON'T USE WITH REAL ODRIVES]
   *
  if ((millis() - timer) >= 1000)
  {
    odrv0.print("\nLoop count: ");
    odrv0.println(loopCount);
    odrv0.println();
    loopCount = 0;
    timer = millis();
  }
  else
  {
    loopCount++;
  }
  ***********************/
}

void handleState(HubStates state)
{
  switch (state)
  {
  case STARTING:
    odrv0.println("Starting...");
    break;

  // Placeholders
  case RUNNING:
    break;
  case STOPPED:
    break;
  case IDLE:
    break;
  }
}