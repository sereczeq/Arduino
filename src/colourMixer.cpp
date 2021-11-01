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
    lcd.print("RED  GREEN  BLUE");
}

int clamp(int n, int currmax, int newmax) {
    float clamped = 0;
    clamped = (float) n / currmax * newmax;
    return (int) clamped;
}

#define POTENTIOMETER A0

int readPotentioMeter() {
    int read = analogRead(POTENTIOMETER);
    int rgb = clamp(read, 1023, 255);
    return rgb;
}

String parseToThreeDigitString(int n) {
    String string = String(n);
    switch (string.length()) {
        case 1:
            return "00" + string;
        case 2:
            return "0" + string;
        default:
            return string;

    }
}
const String redString = "red";
const String greenString = "green";
const String blueString = "blue";
String colors[] = {redString, greenString, blueString};
int values[] = {0, 0, 0};
void display(int n, String color) {
    int position;
    if(color == redString)
    {
        position = 1;
    }
    else if(color == greenString)
    {
        position = 6;
    }
    else
    {
        position = 13;
    }

    lcd.setCursor(position, 1);
    lcd.print(parseToThreeDigitString(n));
}

void displayCursorAtIndex(int index)
{
    int position;
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(5, 1);
    lcd.print(" ");
    lcd.setCursor(12, 1);
    lcd.print(" ");
    if(index == 0)
    {
        position = 0;
    }
    else if (index == 1)
    {
        position = 5;
    }
    else position = 12;
    lcd.setCursor(position, 1);
    lcd.print(">");
}

void changeColorAtIndex(int index)
{
    displayCursorAtIndex(index);
    int read = readPotentioMeter();
    values[index] = read;
}

int currentIndex = 0;
int maxIndex = 2;
void loop() {
    if(green())
    {
        currentIndex++;
        if(currentIndex > maxIndex) currentIndex = 0;
    }
    changeColorAtIndex(currentIndex);
    for(int i = 0; i < 3; i++)
    {
        display(values[i], colors[i]);
    }
    analogWrite(LED_RED, values[0]);
    analogWrite(LED_GREEN, values[1]);
    analogWrite(LED_BLUE, values[2]);
}
