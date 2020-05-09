#include <arduino.h>

#include "buffered_gps.hpp"

static constexpr auto PIN_FIX_FLASH = 12;
static constexpr auto PIN_RED_LED = 13;

auto gps = Scottz0r::BufferedGps();

void setup()
{
    Serial.begin(9600);
    Serial1.begin(9600);

    pinMode(PIN_FIX_FLASH, INPUT);
    digitalWrite(PIN_RED_LED, 0);

    gps.begin();
}

void loop()
{
    gps.process();

    if(gps.sentence_available())
    {
        Serial.write(gps.sentence());
        gps.clear_sentence();
    }

    // TODO: Heartbeat class?
    fix_flashy_light();
}


void fix_flashy_light()
{
    static bool last_state;

    int fix_pin = digitalRead(PIN_FIX_FLASH);

    if(fix_pin && !last_state)
    {
        digitalWrite(PIN_RED_LED, 1);
        last_state = 1;
    }
    else if(!fix_pin && last_state)
    {
        digitalWrite(PIN_RED_LED, 0);
        last_state = 0;
    }
}