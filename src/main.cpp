#include <Arduino.h>
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


int lastFrameEncoderRead = 0;
int previousCounter = -1;
bool wasRedPressed = false;
bool wasGreenPressed = false;
bool redLock = false;
bool greenLock = false;
unsigned long redPressedTime = 0;
unsigned long greenPressedTime = 0;
unsigned long debounceDelay = 50;


void setup() {
    initButtons();
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

void flash()
{
    for(int i = 0; i < lastFrameEncoderRead; i++)
    {
        Serial.println("flashing");
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
}


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

void Task1()
{
    bool isRed = red();
    bool isGreen = green();
    if(redLock && greenLock)
    {
        flash();
        return;
    }
    else if(isRed)
    {
        lastFrameEncoderRead--;
    }
    else if(isGreen)
    {
        lastFrameEncoderRead++;
    }
    if(previousCounter != lastFrameEncoderRead)
    {
        previousCounter = lastFrameEncoderRead;
        Serial.println(lastFrameEncoderRead);
    }
}

unsigned int pressedTime = 0;
unsigned int deltaTime = 0;
bool doOnce;

unsigned int millisInSecond = 1000;
unsigned int secondsInMinute = 60;
unsigned int minutesInHour = 60;

void Task2()
{
    if(redPressed())
    {
        pressedTime += millis() - deltaTime;
        doOnce = true;
    }
    else if(doOnce)
    {
        doOnce = false;
        unsigned int seconds = pressedTime / millisInSecond;
        unsigned int minutes = seconds / secondsInMinute;
        unsigned int hours = minutes / minutesInHour;
        Serial.print(hours);
        Serial.print(":");
        Serial.print(minutes);
        Serial.print(":");
        Serial.print(seconds);
        Serial.print(":");
        Serial.print(pressedTime % millisInSecond);
        Serial.println("");
        pressedTime = 0;
    }

    deltaTime = millis();
}


void Light(int color)
{
    digitalWrite(color, HIGH);
    delay(100);
    digitalWrite(color, LOW);
}

bool pressedLastFrame;

unsigned long lastFrameTime = 0;
unsigned long DeltaTime()
{
    unsigned long time = millis() - lastFrameTime;
    lastFrameTime = millis();
    return time;
}

unsigned long currentPressTime;
int minTime = 2000;

int doubleClickTime = 400;
unsigned long currentDoubleClickTime;
void Task3()
{
    bool isRed = redPressed();
    deltaTime = DeltaTime();
    if(isRed && !pressedLastFrame)
    {
        Light(LED_RED);
    }
    else if(!isRed && pressedLastFrame)
    {
        Light(LED_GREEN);
    }
    if(isRed)
    {
        currentPressTime += deltaTime;
    }
    else
    {
        currentPressTime = 0;
    }
    if(currentPressTime >= minTime)
    {
        Light(LED_RED);
        Light(LED_GREEN);
    }

    if(isRed)
    {
        if(!pressedLastFrame && currentDoubleClickTime < doubleClickTime)
        {
            Light(LED_BLUE);
        }
        else
        {
            currentDoubleClickTime = 0;
        }
    }
    currentDoubleClickTime += deltaTime;
    pressedLastFrame = isRed;
}
void loop() {
Task3();

}