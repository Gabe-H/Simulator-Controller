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

#define FRAME_SIZE 30
#define SCALING_CONSTANT (68.0 / 65535.0)
// #define SCALING_CONSTANT (1)

#define NUM_MOTORS 6

// Motor mapping. Value is the motor number on the Stewart Platform
// according to FlyPT.
#define DRIVE_0_AXIS_0 5
#define DRIVE_0_AXIS_1 0
#define DRIVE_1_AXIS_0 1
#define DRIVE_1_AXIS_1 2
#define DRIVE_2_AXIS_0 3
#define DRIVE_2_AXIS_1 4

class MotorValues
{
public:
    uint16_t rawBytes[6] = {0, 0, 0, 0, 0, 0}; // 0-65535
    float position[6] = {0, 0, 0, 0, 0, 0};    // -40 to 40
};

class SimulatorHub
{
public:
    SimulatorHub(HardwareSerial &odrv0serial);
    // SimulatorHub(HardwareSerial &odrv0serial, HardwareSerial &odrv1serial, HardwareSerial &odrv2serial);

    void setup();
    HubStates loop();

    HubStates processIncomingData();
    void parseMotorValues();
    void updateMotors();

    MotorValues motors;

private:
    void waitForBuffer(uint8_t numBytes);

    HubStates state = BOOT;
    bool gotCmd1 = false;
    bool gotCmd2 = false;

    HardwareSerial &odrv0;

    // HardwareSerial &odrv0;
    // HardwareSerial &odrv1;
    // HardwareSerial &odrv2;
};

#endif // SIMULATORHUB_H