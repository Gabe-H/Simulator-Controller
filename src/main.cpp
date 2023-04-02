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

#ifdef PICO_RP2040

#define odrv0 Serial1

#else

HardwareSerial odrv0(1);

#endif
SimulatorHub hub(odrv0);

unsigned long loopCount = 0;
unsigned long timer = 0;

void handleState(HubStates state);

void setup()
{
  Serial.begin(115200);
  odrv0.begin(115200);

  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(START_BUTTON, INPUT_PULLUP);

  // Setup ODrive serial ports
  hub.setup();
}

void loop()
{
  // Process incoming data and send to ODrives
  if (hub.loop())
  {
    handleState(hub.getState());
  }

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
   * /
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
  /***********************/
}

void handleState(HubStates state)
{
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