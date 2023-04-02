#ifndef SIMULATORSTATES_H
#define SIMULATORSTATES_H

enum FlyPTcommands
{
    FLYPT_CMD = 0xFF,
    // FLYPT_CMD = '$',
    FLYPT_END = '!',
    FLYPT_START = 'S',
    FLYPT_STOP = 'E',
    FLYPT_FRAME = 'F',
};

enum HubStates
{
    IDLE = 1,     // Simulator lowered, ready for rider to get on
    RUNNING = 2,  // Data is being sent from FlyPT
    STARTING = 3, // FlyPT sent start command
    STOPPED = 4,  // FlyPT sent stop command
    READY = 5,    // Start button pressed, simulator ready at 0
};

#endif // SIMULATORSTATES_H