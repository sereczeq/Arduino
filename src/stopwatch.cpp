#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

void initLCD()
{
    lcd.init();
    lcd.clear();
    lcd.backlight();
}

#define BUTTON_RED 2
#define BUTTON_GREEN 4

//digitalRead(BUTTON_RED)
void initButtons()
{
    pinMode(BUTTON_RED, INPUT_PULLUP);
    pinMode(BUTTON_GREEN, INPUT_PULLUP);
}


#define LED_RED 6
#define LED_GREEN 5
#define LED_BLUE 3

void initRGB()
{

    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
}

unsigned long debounceDelay = 50;

bool wasRedPressed;
unsigned long redPressedTime;
bool redLock;
bool redPressed()
{
    bool redPressed = digitalRead(BUTTON_RED) == LOW;
    if(redPressed != wasRedPressed)
    {
        redPressedTime = millis();
    }
    wasRedPressed = redPressed;
    if(millis() - redPressedTime > debounceDelay)
    {

        if(wasRedPressed) return true;
    }
    return false;
}

bool red()
{
    bool pressed = redPressed();
    if(!redLock && pressed)
    {
        redLock = true;
        return true;
    }
    redLock = pressed;
    return false;
}

bool wasGreenPressed;
unsigned long greenPressedTime;
bool greenLock;
bool greenPressed()
{
    bool greenPressed = digitalRead(BUTTON_GREEN) == LOW;
    if(greenPressed != wasGreenPressed)
    {
        greenPressedTime = millis();
    }
    wasGreenPressed = greenPressed;
    if(millis() - greenPressedTime > debounceDelay)
    {
        if(wasGreenPressed) return true;
    }

    return false;
}

bool green()
{
    bool pressed = greenPressed();
    if(!greenLock && pressed)
    {
        greenLock = true;
        return true;
    }
    greenLock = pressed;
    return false;
}

unsigned long lastFrameTime = 0;
unsigned long deltaTime()
{
    unsigned long time = millis() - lastFrameTime;
    lastFrameTime = millis();
    return time;
}



unsigned int millisInSecond = 1000;
unsigned int secondsInMinute = 60;
unsigned int minutesInHour = 60;
String formatTime(unsigned long time)
{
    unsigned int seconds = time / millisInSecond;
    unsigned int minutes = seconds / secondsInMinute;
    seconds -= minutes * secondsInMinute;
    unsigned int hours = minutes / minutesInHour;
    minutes -= hours * minutesInHour;
    String string = String(hours);
    string += ":";
    string += String(minutes);
    string += ":";
    string += String(seconds);
    string += ":";
    string += String(time % millisInSecond);
    return string;
}

bool isCounting;
unsigned long currentTime;
void timer()
{
    if(!isCounting) return;
    currentTime += deltaTime();
    lcd.print(formatTime(currentTime));
    lcd.home();
}

void setup()
{
    initLCD();
    initRGB();
    initButtons();
}

void loop()
{
    bool isGreen = green();
    bool isRed = red();
    if(isGreen)
    {
        isCounting = !isCounting;
    }
    if (isRed)
    {
        isCounting = false;
        currentTime = 0;
        lcd.clear();
    }
    timer();
}
