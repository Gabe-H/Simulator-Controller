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

#define LED_PIN 4

#define LED_BLINK_INTERVAL 750 // ms

HardwareSerial odrv0(1);
SimulatorHub hub(odrv0);

// HardwareSerial odrv0(1);
// HardwareSerial odrv1(2);
// HardwareSerial odrv2(3);
// SimulatorHub hub(odrv0, odrv1, odrv2);

HubStates hubState;
unsigned long lastLedUpdate = 0;
bool blinkState = false;

bool gotCmd = false;

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Setup ODrive serial port
  hub.setup();
}

unsigned long loopCount = 0;
unsigned long timer = 0;

void loop()
{
  // Process incoming data and send to ODrives
  hubState = hub.processIncomingData();

  if (hubState == STARTING)
  {
    odrv0.println("Starting...");
  }

  // Status LED
  if (hubState == RUNNING)
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);

  // Print loop count every second to determine
  // what interval loops we can run
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
}
