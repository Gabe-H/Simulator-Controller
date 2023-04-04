/**
 *
 * SimulatorHub.h
 *
 */

#ifndef SIMULATORHUB_H
#define SIMULATORHUB_H

#include <Arduino.h>
#include <SimulatorStates.h>

#define FRAME_SIZE 30

#define RESET_HEIGHT 34.0
#define SCALING_CONSTANT ((RESET_HEIGHT * 2) / 65535.0)

#define RESET_SPEED 1 // 1 rev/s

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
    uint16_t rawBytes[NUM_MOTORS] = {0, 0, 0, 0, 0, 0}; // 0-65535
    float position[NUM_MOTORS] = {0, 0, 0, 0, 0, 0};    // -RESET_HEIGHT to RESET_HEIGHT
};

class SimulatorHub
{
public:
    SimulatorHub(Stream &odrv0serial, Stream &odrv1serial, Stream &odrv2serial);

    void setup();
    bool loop();

    HubStates getState() { return state; }
    bool stateChange();

    bool processIncomingData();
    void parseMotorValues();
    void updateMotors();

    void stopSimulator();
    void startSimulator();

    MotorValues motors;

private:
    void waitForBuffer(uint8_t numBytes);

    HubStates state = IDLE;
    HubStates oldState;
    bool gotCmd1 = false;
    bool gotCmd2 = false;

    Stream &odrv0;
    Stream &odrv1;
    Stream &odrv2;
};

#endif // SIMULATORHUB_H