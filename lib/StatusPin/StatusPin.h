#ifndef INTERRUPTLED_H
#define INTERRUPTLED_H

#include <Arduino.h>

enum LedModes
{
    LED_OFF,
    LED_ON,
    LED_BLINK
};

class StatusPin
{
public:
    StatusPin(int pin);

    void setup();
    void loop();
    void setDelay(unsigned long delay) { blink_delay = delay; }
    void setMode(LedModes mode);

private:
    int _pin;
    bool ledState = false;
    bool _doBlink = true;
    LedModes _mode = LED_OFF;
    unsigned long blink_delay = 1000;
    unsigned long timer = 0;
};

#endif