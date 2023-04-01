#include "SimulatorHub.h"

SimulatorHub::SimulatorHub(HardwareSerial &odrv0serial) : odrv0(odrv0serial)
{
}

// SimulatorHub::SimulatorHub(HardwareSerial &odrv0serial, HardwareSerial &odrv1serial, HardwareSerial &odrv2serial) : odrv0(odrv0serial), odrv1(odrv1serial), odrv2(odrv2serial)
// {
// }

void SimulatorHub::setup()
{
    odrv0.begin(BAUD_RATE_OUT);

    for (int i = 0; i < NUM_MOTORS; i++)
    {
        motors.rawBytes[i] = 0;
        motors.position[i] = 0;
    }
}

HubStates SimulatorHub::loop()
{
    return processIncomingData();
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
        {
            // Next character is the command
            gotCmd = true;
        }
        else
        {
            // Ignore the character
            gotCmd = false;
        }
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

        // Convert the motor value to a float between -34 and 34
        motors.position[motorNum] = (SCALING_CONSTANT * float(motorValue)) - 34;
    }
}

/* Send the motor values at a time to their ODrive
 * Example frame:
 * q 0 <uin16_t>\n
 * q 1 <uin16_t>\n
 */
void SimulatorHub::updateMotors()
{
    char frame[FRAME_SIZE];
    const char *fmt = "q 0 %.2f\rq 1 %.2f\r";

    // ODrive 0 => 6, 1
    sprintf(frame, fmt, motors.position[DRIVE_0_AXIS_0], motors.position[DRIVE_0_AXIS_1]);
    odrv0.print(frame);

    // ODrive 1 => 2, 3
    sprintf(frame, fmt, motors.position[DRIVE_1_AXIS_0], motors.position[DRIVE_1_AXIS_1]);
    // odrv1.print(frame);
    odrv0.print(frame);

    // ODrive 2 => 4, 5
    sprintf(frame, fmt, motors.position[DRIVE_2_AXIS_0], motors.position[DRIVE_2_AXIS_1]);
    // odrv2.print(frame);
    odrv0.print(frame);

    odrv0.flush();
}

void SimulatorHub::waitForBuffer(uint8_t numBytes)
{
    while (Serial.available() < numBytes)
        ;
}