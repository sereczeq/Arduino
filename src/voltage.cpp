//
// Created by kubas on 27.10.2021.
//
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define ENCODER1 A2
#define ENCODER2 A3

void initLCD() {
    lcd.init();
    lcd.clear();
    lcd.backlight();
}

#define BUTTON_RED 2
#define BUTTON_GREEN 4

//digitalRead(BUTTON_RED)
void initButtons() {
    pinMode(BUTTON_RED, INPUT_PULLUP);
    pinMode(BUTTON_GREEN, INPUT_PULLUP);
}


#define LED_RED 6
#define LED_GREEN 5
#define LED_BLUE 3

void initRGB() {

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
}

unsigned long debounceDelay = 50;

bool wasRedPressed;
unsigned long redPressedTime;
bool redLock;

bool redPressed() {
    bool redPressed = digitalRead(BUTTON_RED) == LOW;
    if (redPressed != wasRedPressed) {
        redPressedTime = millis();
    }
    wasRedPressed = redPressed;
    if (millis() - redPressedTime > debounceDelay) {

        if (wasRedPressed) return true;
    }
    return false;
}

bool red() {
    bool pressed = redPressed();
    if (!redLock && pressed) {
        redLock = true;
        return true;
    }
    redLock = pressed;
    return false;
}

bool wasGreenPressed;
unsigned long greenPressedTime;
bool greenLock;

bool greenPressed() {
    bool greenPressed = digitalRead(BUTTON_GREEN) == LOW;
    if (greenPressed != wasGreenPressed) {
        greenPressedTime = millis();
    }
    wasGreenPressed = greenPressed;
    if (millis() - greenPressedTime > debounceDelay) {
        if (wasGreenPressed) return true;
    }

    return false;
}

bool green() {
    bool pressed = greenPressed();
    if (!greenLock && pressed) {
        greenLock = true;
        return true;
    }
    greenLock = pressed;
    return false;
}


void setup() {
    initLCD();
    initRGB();
    initButtons();
    analogWrite(LED_RED, 30);
    analogWrite(LED_BLUE, 30);

    lcd.clear();
    lcd.home();
    lcd.print("Potentiometer");
    lcd.setCursor(0, 1);
    lcd.print("V=");
    lcd.setCursor(16-8, 1);
    lcd.print("ACD=");
}
#define POTENTIOMETER A0

int readPotentioMeter() {
    return analogRead(POTENTIOMETER);
}

String parseToFourDigitString(int n) {
    String string = String(n);
    switch (string.length()) {
        case 1:
            return "000" + string;
        case 2:
            return "00" + string;
        case 3:
            return "0" + string;
        default:
            return string;
    }
}

float clamp(int n, float currmax, float newmax) {
    float clamped = 0;
    clamped = (float) n / currmax * newmax;
    return clamped;
}

void loop()
{
    lcd.setCursor(16-4, 1);
    lcd.print(parseToFourDigitString(readPotentioMeter()));
    lcd.setCursor(2, 1);
    lcd.print(String(clamp(readPotentioMeter(), 1023, 5), 2));
}
