#include <StatusPin.h>

StatusPin::StatusPin(int pin)
{
    _pin = pin;
}

void StatusPin::setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
}

void StatusPin::setMode(LedModes mode)
{
    switch (mode)
    {
    case LED_OFF:
        digitalWrite(LED_BUILTIN, LOW);
        break;
    case LED_ON:
        digitalWrite(LED_BUILTIN, HIGH);
        break;
    case LED_BLINK:
        _doBlink = true;
        timer = millis() - blink_delay;
        ledState = true;
        break;
    }

    _mode = mode;
}

void StatusPin::loop()
{
    if (_mode != LED_BLINK)
        return;

    if ((millis() - timer) < blink_delay)
        return; // Don't do anything if the timer isn't ready

    digitalWrite(LED_BUILTIN, ledState);
    ledState = !ledState;
    timer = millis();
}
