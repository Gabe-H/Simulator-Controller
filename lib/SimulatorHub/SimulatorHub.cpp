#include "SimulatorHub.h"

SimulatorHub::SimulatorHub()
{
}

SimulatorHub::~SimulatorHub()
{
}

void SimulatorHub::setup()
{
    Serial.begin(BAUD_RATE_IN);

    for (int i = 0; i < NUM_MOTORS; i++)
    {
        motors.rawBytes[i] = 0;
        motors.position[i] = 0;
    }
}

HubStates SimulatorHub::loop()
{
    HubStates state = processIncomingData();
    parseMotorValues();
    updateMotors();

    return state;
}

HubStates SimulatorHub::processIncomingData()
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

    HubStates state;
    // Read the incoming data
    while (Serial.available() > 0)
    {
        char c = Serial.read();
        if (gotCmd)
        {
            bool retain = false;
            switch (c)
            {
            case FLYPT_CMD:
                // Next character is the command
                retain = true;
                break;
            case FLYPT_START:
                // TODO: Start the motors
                state = STARTING;
                break;

            case FLYPT_STOP:
                // TODO: Stop the motors
                state = IDLE;
                break;

            case FLYPT_FRAME:
                // Read the next x bytes and parse them as motor values
                state = RUNNING;
                parseMotorValues();
                break;

            default:
                // Ignore the character
                break;
            }
            gotCmd = retain; // If we're retaining the command, keep it true
        }
        else if (c == FLYPT_CMD)
            // Next character is the command
            gotCmd = true;
        else
            // Ignore the character
            gotCmd = false;
    }

    return state;
}

void SimulatorHub::parseMotorValues()
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
        motors.position[motorNum] = ((0.0012207) * float(motorValue)) - 40.0;
    }
}

void SimulatorHub::updateMotors()
{
}

void SimulatorHub::waitForBuffer(uint8_t numBytes)
{
    while (Serial.available() < numBytes)
        ;
}