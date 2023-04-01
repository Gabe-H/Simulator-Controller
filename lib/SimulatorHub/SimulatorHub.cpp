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
    // odrv1.begin(BAUD_RATE_OUT);
    // odrv2.begin(BAUD_RATE_OUT);

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
     * All commands are prefixed with two 0xFF characters
     *
     * Frames are sent beginning with a 'F' character, followed by
     * an integer representing the motor number. The next 2 bytes
     * are the position of the motor (unsigned 16-bit integer, little
     * endian). The frame is closed with a '!' character.
     *
     * Example frame:
     *  $ \xFF \xFF F 1 \x01 \x23 2 \x04 \x56 3 \x08 \x79 4 \x0C \x9A 5 \x0E \xBC 6 \x10 \xDE !
     *
     * Each frame is 22 bytes long.
     */

    // Read the incoming data
    if (Serial.available() > 0)
    {
        char c = Serial.read();
        if (c == FLYPT_CMD)
        {
            // Next character is the command
            if (gotCmd1)
                gotCmd2 = true;
            else
                gotCmd1 = true;
        }
        else if (gotCmd2)
        {
            switch (c)
            {
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
            gotCmd1 = false; // Reset the command booleans
            gotCmd2 = false;
        }
        else
        {
            // Ignore the character
            gotCmd2 = false;
            gotCmd1 = false;
        }
    }

    return state;
}

void SimulatorHub::parseMotorValues()
{
    uint8_t motorNum = 0;

    while (true)
    {
        waitForBuffer(1); // 1 byte for motor number or end of frame

        char c = Serial.read();

        // First, check if the frame is complete
        if (c == FLYPT_END)
        {
            // Frame is done. Update motors
            updateMotors();
            break;
        }

        // The character is the motor number
        // Note: char starts at 1, motor ndx starts at 0
        motorNum = c - '1';

        waitForBuffer(2); // Motor value is unsigned 16-bit integer

        byte msb = Serial.read(); // Most significant byte
        byte lsb = Serial.read(); // Least significant byte

        uint16_t motorValue = (msb << 8) | lsb; // Combine the bytes uint16_t

        motors.rawBytes[motorNum] = motorValue; // Store the raw bytes (for debugging)

        // Convert the motor value to a float between +/- REST_HEIGHT (34.0)
        motors.position[motorNum] = (SCALING_CONSTANT * float(motorValue)) - REST_HEIGHT;
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
    const char *fmt = "q 0 %.2f\rq 1 %.2f\r"; // Carriage return for easier debugging
    // const char *fmt = "q 0 %.2f\nq 1 %.2f\n"; // Use this for actual ODrives

    // ODrive 0 => Motors 6, 1
    sprintf(frame, fmt, motors.position[DRIVE_0_AXIS_0], motors.position[DRIVE_0_AXIS_1]);
    odrv0.print(frame);

    // ODrive 1 => Motors 2, 3
    sprintf(frame, fmt, motors.position[DRIVE_1_AXIS_0], motors.position[DRIVE_1_AXIS_1]);
    // odrv1.print(frame);
    odrv0.print(frame);

    // ODrive 2 => Motors 4, 5
    sprintf(frame, fmt, motors.position[DRIVE_2_AXIS_0], motors.position[DRIVE_2_AXIS_1]);
    // odrv2.print(frame);
    odrv0.print(frame);

    // odrv0.flush(); // Clear outgoing buffer
}

void SimulatorHub::waitForBuffer(uint8_t numBytes)
{
    while (Serial.available() < numBytes)
        ;
}