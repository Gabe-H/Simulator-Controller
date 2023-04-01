#ifndef SIMULATORSTATES_H
#define SIMULATORSTATES_H

enum FlyPTcommands
{
    FLYPT_CMD = 0xFF,
    FLYPT_END = '!',
    FLYPT_START = 'S',
    FLYPT_STOP = 'E',
    FLYPT_FRAME = 'F',
};

enum HubStates
{
    BOOT = 0,
    IDLE = 1,
    RUNNING = 2,
    STARTING = 3,
};

#endif // SIMULATORSTATES_H