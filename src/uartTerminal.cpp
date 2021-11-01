//
// Created by kubas on 27.10.2021.
//

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define LED_RED 6
#define LED_GREEN 5
#define LED_BLUE 3

void initRGB() {

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
}

void setup()
{
    initRGB();
    Serial.begin(9600);
}

bool contains(String one, String other)
{
    return (one.indexOf(other) > 0);
}

void loop()
{
    if (Serial.available() > 0) {
        String res = Serial.readString();
        Serial.print("I received: ");
        Serial.println(res);
        if(contains(res, "jej"))
        {
            Serial.println("got it");
        }
    }
}