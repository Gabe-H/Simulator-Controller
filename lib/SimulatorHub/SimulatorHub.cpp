#include "SimulatorHub.h"

SimulatorHub::SimulatorHub(Stream &odrv0serial, Stream &odrv1serial, Stream &odrv2serial) : odrv0(odrv0serial), odrv1(odrv1serial), odrv2(odrv2serial)
{
}

void SimulatorHub::setup()
{
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        motors.rawBytes[i] = 0;
        motors.position[i] = 0;
    }
}

// Return true if the state has changed
bool SimulatorHub::loop()
{
    return processIncomingData();
}

bool SimulatorHub::stateChange()
{
    if (state != oldState)
    {
        oldState = state;
        return true;
    }

    return false;
}

bool SimulatorHub::processIncomingData()
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
     *  $ \xFF \xFF 'F' '1' \xXX \xXX '2' \xXX \xXX '3' \xXX \xXX '4' \xXX \xXX '5' \xXX \xXX '6' \xXX \xXX !
     *
     * Each frame is 22 bytes long.
     */

    bool running = false;

    // Read the incoming data
    if (Serial.available() > 0)
    {
        byte c = Serial.read();
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
                // For implementation if needed
                state = STARTING;
                break;

            case FLYPT_STOP:
                // For implementation if needed
                state = STOPPED;
                break;

            case FLYPT_FRAME:
                // Read the next x bytes and parse them as motor values
                if (!sendOutput) // Disable if stopped
                    break;

                state = RUNNING;
                parseMotorValues();
                running = true;
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

    return running;
}

void SimulatorHub::parseMotorValues()
{
    uint8_t motorNum = 0;

    // Run maximum of 6 times (one for each motor)
    for (int i = 0; i < NUM_MOTORS; i++)
    {
        waitForBuffer(1); // 1 byte for motor number or end of frame

        char c = Serial.read();

        // The character is the motor number
        // Note: char starts at 1, motor ndx starts at 0
        motorNum = c - '1';

        waitForBuffer(2); // Motor value is unsigned 16-bit integer

        byte msb = Serial.read(); // Most significant byte
        byte lsb = Serial.read(); // Least significant byte

        uint16_t motorValue = (msb << 8) | lsb; // Combine the bytes to uint16_t

        motors.rawBytes[motorNum] = motorValue; // Store the raw bytes (for debugging)

        // Convert the motor value to a float between +/- RESET_HEIGHT (34.0)
        motors.position[motorNum] = (SCALING_CONSTANT * float(motorValue)) - RESET_HEIGHT;

        /** TODO: Verify values? I'm not sure if we need to though since the defines are constants
        and the max/min values are known to be within the range of the uint16_t */

        // Check next byte to see if it's the end of the frame before moving on
        if (Serial.peek() == FLYPT_END_FRAME)
        {
            // Frame is done. Update motors
            updateMotors();
            break;
        }
        // Else keep reading values
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
    const char *fmt = "q 0 %s q 1 %s\r"; // Carriage return for easier debugging
    // const char *fmt = "q 0 %s\nq 1 %s\n"; // Use this for actual ODrives

    char motor0[20];
    char motor1[20];

    // ODrive 0 => Motors 6, 1
    dtostrf(motors.position[DRIVE_0_AXIS_0], 0, 2, motor0); // 0: minumum string length, 2: number of decimal places
    dtostrf(motors.position[DRIVE_0_AXIS_1], 0, 2, motor1);
    sprintf(frame, fmt, motor0, motor1);
    odrv0.print(frame);

    // ODrive 1 => Motors 2, 3
    dtostrf(motors.position[DRIVE_1_AXIS_0], 0, 2, motor0);
    dtostrf(motors.position[DRIVE_1_AXIS_1], 0, 2, motor1);
    sprintf(frame, fmt, motor0, motor1);
    odrv1.print(frame);

    // ODrive 2 => Motors 4, 5
    dtostrf(motors.position[DRIVE_2_AXIS_0], 0, 2, motor0);
    dtostrf(motors.position[DRIVE_2_AXIS_1], 0, 2, motor1);
    sprintf(frame, fmt, motor0, motor1);
    odrv2.print(frame);
}

void SimulatorHub::waitForBuffer(uint8_t numBytes)
{
    while (Serial.available() < numBytes)
        ;
}

void SimulatorHub::stopSimulator()
{
    state = IDLE;

    delay(1000);

    // Move all motors to rest position (NOTE NEGATIVE SIGN)
    const char *restCmdFmt = "q 0 -%u %u q 1 -%u %u\r";
    odrv0.println(); // New line for easier debugging
    odrv1.println(); // New line for easier debugging
    odrv2.println(); // New line for easier debugging

    // const char *restCmdFmt = "q 0 -%u %u\nq 1 -%u %u\n";
    /** TODO: investigate using trajectory control for smoother operation */

    char restCmd[FRAME_SIZE];

    sprintf(restCmd, restCmdFmt, int(RESET_HEIGHT), RESET_SPEED, int(RESET_HEIGHT), RESET_SPEED);

    odrv0.print(restCmd);
    odrv1.print(restCmd);
    odrv2.print(restCmd);

    setOutput(false); // Stop data flow
}

void SimulatorHub::startSimulator()
{
    // Move all motors to neutral position
    const char *zeroCmdFmt = "q 0 0 %u q 1 0 %u\r\n";
    odrv0.println(); // New line for easier debugging
    odrv1.println(); // New line for easier debugging
    odrv2.println(); // New line for easier debugging

    // const char *zeroCmd = "q 0 0 %u\nq 1 0 %u\n";
    /** TODO: investigate using trajector control for smoother operation */

    char zeroCmd[FRAME_SIZE];

    sprintf(zeroCmd, zeroCmdFmt, RESET_SPEED, RESET_SPEED);

    odrv0.print(zeroCmd);
    odrv1.print(zeroCmd);
    odrv2.print(zeroCmd);

    /** TODO: Wait for motors to reach neutral position */

    state = READY;

    setOutput(true); // Start data flow
}