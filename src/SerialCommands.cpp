//
// Created by kubas on 10.11.2021.
//
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void initLCD()
{
    lcd.init();
    lcd.clear();
    lcd.backlight();
}

#define LED_RED 6
#define LED_GREEN 5
#define LED_BLUE 3

//region init
void initRGB()
{

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
}


void initLED()
{
    pinMode(LED_BUILTIN, OUTPUT);
}

void initSerial()
{
    Serial.begin(9600);
    Serial.println("Serial initialized");
}
//endregion

unsigned long lastGreenDebounceTime = 0;
unsigned long lastRedDebounceTime = 0;
unsigned long debounceDelay = 50;


void setup() {
    initLCD();
    initRGB();
    initLED();
    initSerial();
}

bool blinking = false;
int blinkDelay = 1;
void readSerial()
{

    while (Serial.available() > 0) {
        String command = Serial.readStringUntil(' ');
        String arg1 = Serial.readStringUntil(' ');
        String arg2 = Serial.readStringUntil('\0');

        Serial.println("Reading command: " + command + " " + arg1 + " " + arg2);

        if (command != "LED") {
            Serial.println("[ERROR]: " + command + " is an unrecognized command");
            return;
        }

        if (arg1.startsWith("ON")) {
            Serial.println("Turning led ON");
            digitalWrite(LED_BLUE, HIGH);
            blinking = false;
            return;
        }
        if (arg1.startsWith("OFF")) {
            Serial.println("Turning led OFF");
            digitalWrite(LED_BLUE, LOW);
            blinking = false;
            return;
        }
        if (arg1.startsWith("BLINK") && arg2.length() > 0) {
            if (arg2.length() > 0)
            {
                blinking = true;
                blinkDelay = arg2.toInt() * 1000;
                Serial.println("Setting led to blink every " + String(blinkDelay) + "milliseconds");
                return;
            }
            Serial.println("[ERROR]: blink delay " + arg2 + " couldn't be parsed");
            return;
        }
        Serial.println("[ERROR]: argument " + arg1 + " is not a valid argument");
    }
}


unsigned long currentBlinkTime;
void blink()
{
    if(!blinking) return;
    if(millis() - currentBlinkTime > blinkDelay)
    {
        currentBlinkTime = millis();

        int ledState = digitalRead(LED_BLUE) == HIGH? LOW : HIGH;
        digitalWrite(LED_BLUE, ledState);
    }

}
void loop() {
    readSerial();
    blink();


}