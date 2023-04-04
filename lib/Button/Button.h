#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button
{
public:
    Button(int pin)
    {
        this->pin = pin;
        pinMode(pin, INPUT_PULLUP);
    }

    // Returns true when the button was pressed
    bool handle()
    {
        int state = digitalRead(pin);

        // Button is pressed
        if (state == LOW && oldState == HIGH)
        {
            // Catch bouncing
            if ((millis() - lastDebounceTime) < debounce)
                return false;

            oldState = state;
            lastDebounceTime = millis();
            return true;
        }
        else
        {
            oldState = state;
            return false;
        }
    }

private:
    int pin;
    int oldState = HIGH;
    unsigned long debounce = 100; // ms
    unsigned long lastDebounceTime = 0;
};

#endif // BUTTON_H