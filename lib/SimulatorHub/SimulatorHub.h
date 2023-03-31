/**
 *
 * SimulatorHub.h
 *
 */

#ifndef SIMULATORHUB_H
#define SIMULATORHUB_H

#include <Arduino.h>
#include <SimulatorStates.h>

#define BAUD_RATE_IN 115200
#define BAUD_RATE_OUT 115200

#define NUM_MOTORS 6

class MotorValues
{
public:
    uint16_t rawBytes[6] = {0, 0, 0, 0, 0, 0}; // 0-65535
    float position[6] = {0, 0, 0, 0, 0, 0};    // -40 to 40
};

class SimulatorHub
{
public:
    SimulatorHub();
    ~SimulatorHub();

    void setup();
    HubStates loop();

    HubStates processIncomingData();
    void parseMotorValues();
    void updateMotors();

    MotorValues motors;

private:
    void waitForBuffer(uint8_t numBytes);

    bool gotCmd = false;
};

#endif // SIMULATORHUB_H