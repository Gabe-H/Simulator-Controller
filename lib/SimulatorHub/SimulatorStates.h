#ifndef SIMULATORSTATES_H
#define SIMULATORSTATES_H

enum FlyPTcommands
{
    FLYPT_CMD = '$',
    FLYPT_END = '!',
    FLYPT_START = 'S',
    FLYPT_STOP = 'E',
    FLYPT_FRAME = 'F',
    FLYPT_AXIS = 'A',
};

enum HubStates
{
    BOOT = 0,
    IDLE = 1,
    RUNNING = 2,
    STARTING = 3,
};

#endif // SIMULATORSTATES_H