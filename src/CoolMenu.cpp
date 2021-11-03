//
// Created by kubas on 01.11.2021.
//

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

//region init
#define BUTTON_DEBOUNCE 50
#define ENCODER_DEBOUNCE 100

LiquidCrystal_I2C lcd(0x27, 16, 2);

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

#define ENCODER_A A2   //PCINT10, PORT C, PCIE1, PCMSK1
#define ENCODER_B A3   //PCINT11, PORT C, PCIE1, PCMSK1

void initEncoder()
{
    pinMode(ENCODER_A, INPUT_PULLUP);
    pinMode(ENCODER_B, INPUT_PULLUP);
}


void initLED()
{
    pinMode(LED_BUILTIN, OUTPUT);
}

void setupPCI()
{
    //Here we activate pin change interruptions on pin A2
    PCICR |= (1 << PCIE1);      //enable PCMSK1 scan
    PCMSK1 |= (1 << PCINT10);    //Pin A interrupt. Set pin A2 to trigger an interrupt on state change.
}
//endregion


//region buttons
// RED - 0, GREEN - 1
bool wasButtonPressed[] = {false, false};
unsigned long buttonPressedTime[] = {0, 0};
bool buttonLock[] = {false, false};

int buttonPinToArrayIndex(int pin)
{
    return pin == BUTTON_RED ? 0 : 1;
}

bool buttonPressed(int pin)
{
    int index = buttonPinToArrayIndex(pin);
    bool pressed = digitalRead(pin) == LOW;
    unsigned long currentMillis = millis();
    if (pressed != !wasButtonPressed[index])
    {
        buttonPressedTime[index] = currentMillis;
    }
    wasButtonPressed[index] = pressed;

    if (currentMillis - buttonPressedTime[index] > BUTTON_DEBOUNCE)
    {
        if (wasButtonPressed[index]) return true;
    }
    return false;
}

bool buttonPressedThisFrame(int pin)
{
    int index = buttonPinToArrayIndex(pin);
    bool pressed = buttonPressed(pin);
    if (!buttonLock[index] && pressed)
    {
        buttonLock[index] = true;
        return true;
    }
    buttonLock[index] = pressed;
    return false;
}
//endregion

//region old working buttons

bool wasRedPressed = false;
bool wasGreenPressed = false;
bool redLock = false;
bool greenLock = false;
unsigned long redPressedTime = 0;
unsigned long greenPressedTime = 0;
unsigned long debounceDelay = 50;

bool greenPressed()
{
    bool greenPressed = digitalRead(BUTTON_GREEN) == LOW;
    if (greenPressed != wasGreenPressed)
    {
        greenPressedTime = millis();
    }
    wasGreenPressed = greenPressed;
    if (millis() - greenPressedTime > debounceDelay)
    {
        if (wasGreenPressed) return true;
    }

    return false;
}

bool green()
{
    bool pressed = greenPressed();
    if (!greenLock && pressed)
    {
        greenLock = true;
        return true;
    }
    greenLock = pressed;
    return false;
}

bool redPressed()
{
    bool redPressed = digitalRead(BUTTON_RED) == LOW;
    if (redPressed != wasRedPressed)
    {
        redPressedTime = millis();
    }
    wasRedPressed = redPressed;
    if (millis() - redPressedTime > debounceDelay)
    {

        if (wasRedPressed) return true;
    }
    return false;
}

bool red()
{
    bool pressed = redPressed();
    if (!redLock && pressed)
    {
        redLock = true;
        return true;
    }
    redLock = pressed;
    return false;
}

//endregion


String selector = "->";

//region Defining MenuWithOptions class
struct MenuWithOptions
{
    MenuWithOptions(const String &optionName, int subMenusSize, MenuWithOptions *subMenus) : name(optionName),
                                                                                             children(subMenus),
                                                                                             size(subMenusSize)
    { Serial.println("Calling constructor of " + name); }

    MenuWithOptions(const String &name, void (*func)()) : name(name), func(func)
    { Serial.println("Calling constructor of " + name); }

    MenuWithOptions()
    { Serial.println("Calling constructor of " + name); }

private:
    String name;
    MenuWithOptions *children;
    int size = 0;
    int currentIndex{};
    int checkint = 1;

    void (*func)(){};


private:

    bool hasSubMenuOpened{};
private:
    void setCurrentIndex(int newIndex)
    {
        if (newIndex < 0) newIndex = size - 1;
        else if (newIndex >= size) newIndex = 0;
        currentIndex = newIndex;
    }

    bool hasChildren() const
    {
        Serial.println(name + "size is " + size);
        return size > 0;
    }

    String *getNames() const
    {
        String names[1 + size];
        names[1] = name;
        for (int i = 0; i < size; i++)
        {
            names[1 + i] = children[i].name;
        }

        return names;
    }

public:
    /// next Index - Increment the child index of the deepest submenu still having children
    int nextIndex()
    {
        // If menu doesn't have sub menus opened, return 1, incrementing the index of parent
        if (!hasSubMenuOpened)
        {
            return 1;
        }
        // Change menus index by recursively returned amount, if child changed indexes of its children, don't change indexes

        int childReturned = children[currentIndex].nextIndex();
        setCurrentIndex(currentIndex + childReturned);
        if (childReturned == 1)
        {
            Serial.println(name + " " + selector + " " + children[currentIndex].name);
        }
//        Serial.println(name + " is at index " + currentIndex);
        return 0;
    }

    /// previous Index - Decrement the child index of the deepest submenu still having children
    int previousIndex()
    {
        // If menu doesn't have sub menus opened, return -1, decrementing the index of parent
        if (!hasSubMenuOpened)
        {
            return -1;
        }
        // Change menus index by recursively returned amount, if child changed indexes of its children, don't change indexes
        int childReturned = children[currentIndex].previousIndex();
        setCurrentIndex(currentIndex + childReturned);
        if (childReturned == -1)
        {
            Serial.println(name + " " + selector + " " + children[currentIndex].name);
        }
        return 0;
    }

    /// enter sub menu - Changes display (title and sub options) or executes the chosen option. Recursively calls enter sub menu, until called menu doesn't have children, execute() is called.
    String *enterSubMenu()
    {
        Serial.println("Enter " + name);
        // If menu doesn't have children, and enter sub menu was called on it, that means it's the last in the chain, and should execute its method
        // Returns null pointer so parent can properly return getNames()
        if (!hasChildren())
        {
            Serial.println("Running function");
            func();
            return nullptr;
        }

        // If menu has opened child, call enter sub menu on the child.
        // If the child returned null, meaning it doesn't have children (so doesn't have names to display), display names
        Serial.println(name + "'s hasSubMenuOpened is set to " + hasSubMenuOpened);
        if (hasSubMenuOpened)
        {
            Serial.println(name + " has children opened, passing through");
            String *names = children[currentIndex].enterSubMenu();
            if (names == nullptr)
            {
                return getNames();
            }
            return names;
        }
        // If menu doesn't have opened children, but has children, open sub menu (change the bool) and display names
        hasSubMenuOpened = true;
        Serial.println(name + "'s hasSubMenuOpened is set to " + checkint++);
        Serial.println("Entered a child");
        Serial.println(name);
        return getNames();
    }

    String *exitSubMenu()
    {
        Serial.println("Exiting sub menu " + name);
        if (!hasChildren())
        {
            Serial.println(name + " returning null");
            return nullptr;
        }

        if (hasSubMenuOpened)
        {
            String *childName = children[currentIndex].exitSubMenu();
            if (childName == nullptr)
            {
                Serial.println(name + " setting to false");
                hasSubMenuOpened = false;
                return &name;
            }
            Serial.println(name);
            return &name;
        }

        return nullptr;

        // Can do that without checking whether menu has children. Method will never get called for menu not having children

//        if (hasSubMenuOpened)
//        {
//            String *childName = children[currentIndex].exitSubMenu();
//            if (childName == nullptr)
//            {
//                hasSubMenuOpened = false;
//                return getNames();
//            }
//
//            if(!children[currentIndex].hasChildren())
//            {
//                hasSubMenuOpened = false;
//                return nullptr;
//            }
//
//            return childName;
//        }
//        return nullptr;
    }
};
//endregion

//region Defining specific methods called by MenuWithOptions

int ledPower[3] = {255, 255, 255};

void ledOn()
{
    analogWrite(LED_RED, ledPower[0]);
    analogWrite(LED_GREEN, ledPower[1]);
    analogWrite(LED_BLUE, ledPower[2]);
}

void ledOff()
{
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 0);
}

void RedLed()
{
    int value = ledPower[0];
    while (greenPressed())
    {
        value++;
        if (value > 255) value -= 255;
        ledPower[0] = value;
        Serial.println(value);
        analogWrite(LED_RED, value);
    }
}

void GreenLed()
{
    int value = ledPower[1];
    while (greenPressed())
    {
        value++;
        if (value > 255) value -= 255;
        ledPower[1] = value;
        Serial.println(value);
        analogWrite(LED_GREEN, value);
    }
}

void BlueLed()
{
    int value = ledPower[2];
    while (greenPressed())
    {
        value++;
        if (value > 255) value -= 255;
        ledPower[2] = value;
        Serial.println(value);
        analogWrite(LED_BLUE, value);
    }
}

void backLightOn()
{
    lcd.backlight();
}

void backLightOff()
{
    lcd.noBacklight();
}

const String celcius = "C";
const String farenheit = "F";
String currentTemperatureFormat = celcius;

// (X°C × 9/5) + 32 = Y°F
float calculateTemperature(float temperature)
{
    if (currentTemperatureFormat == celcius) return temperature;
    return temperature * 9 / 5 + 32;
}

void temperatureIn()
{
    float temperature = calculateTemperature(100);
    Serial.println(temperature);
}

void temperatureOut()
{
    float temperature = calculateTemperature(18);
    Serial.println(temperature);
}

void changeTemperatureFormatToCelcius()
{
    currentTemperatureFormat = celcius;
}

void changeTemperatureFormatToFarenheit()
{
    currentTemperatureFormat = farenheit;
}

void changeSelectorToArrow()
{
    selector = "->";
}

void changeSelectorToDash()
{
    selector = "-";
}

void printAbout()
{
    Serial.println("Made by Jakub Seredyński");
}

void empty()
{

}
//endregion

volatile int lastFrameEncoderRead = 0;
volatile long lastInterruptTime = 0;

//Handler for PC Interrupt on bank C (ports from A0 to A5)
ISR(PCINT1_vect)
{
    int currentStateA = digitalRead(ENCODER_A);
    if (currentStateA != LOW)
        return;

    noInterrupts();
    unsigned long interruptTime = millis();

    if (interruptTime - lastInterruptTime > ENCODER_DEBOUNCE)
    {
        if (digitalRead(ENCODER_B) == LOW)
            lastFrameEncoderRead = 1;
        else
            lastFrameEncoderRead = -1;

        lastInterruptTime = interruptTime;
    }
    interrupts();
}

MenuWithOptions mainMenu;

void initMainMenu()
{
    mainMenu = MenuWithOptions("Main Menu", 4,
                               new (MenuWithOptions[4]){
                                       {"LED Options", 4,
                                               new (MenuWithOptions[4]) {
                                                       {"Power", 2,
                                                               new (MenuWithOptions[2]) {
                                                                       {"On",  ledOn},
                                                                       {"Off", ledOff}
                                                               }},
                                                       {"Red",   RedLed},
                                                       {"Green", GreenLed},
                                                       {"Blue",  BlueLed}
                                               }},
                                       {"Display",     2,
                                               new (MenuWithOptions[2]) {
                                                       {"Backlight", 2,
                                                               new (MenuWithOptions[2]) {
                                                                       {"On",  backLightOn},
                                                                       {"Off", backLightOff}}},
                                                       {"Selector",  3,
                                                               new (MenuWithOptions[3]) {
                                                                       {">",             changeSelectorToArrow},
                                                                       {"-",             changeSelectorToDash},
                                                                       {"<custom char>", empty}
                                                               }}
                                               }},
                                       {"Temperature", 3,
                                               new (MenuWithOptions[3]) {
                                                       {"Sensor IN",  temperatureIn},
                                                       {"Sensor OUT", temperatureOut},
                                                       {"Units",      2,
                                                               new (MenuWithOptions[2]) {
                                                                       {"C", changeTemperatureFormatToCelcius},
                                                                       {"F", changeTemperatureFormatToFarenheit}
                                                               }}
                                               }},
                                       {"About",       printAbout}
                               });
    mainMenu.enterSubMenu();
}


void setup()
{
    initLCD();
    initButtons();
    initRGB();
    initEncoder();
    initLED();
    Serial.begin(9600);
    setupPCI();
    initMainMenu();
}


void loop()
{
    static int encoderRead;
    noInterrupts();
    encoderRead = lastFrameEncoderRead;
    lastFrameEncoderRead = 0;
    interrupts();
    if (encoderRead == 1)
    {
        mainMenu.nextIndex();
    } else if (encoderRead == -1)
    {
        mainMenu.previousIndex();
    }
    if (green())
    {
        mainMenu.enterSubMenu();
    } else if (red())
    {
        mainMenu.exitSubMenu();
    }
}