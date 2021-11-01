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


//region Defining MenuWithOptions class
struct MenuWithOptions
{
    MenuWithOptions(const String &optionName, int subMenusSize, MenuWithOptions *subMenus) : name(optionName),
                                                                                             children(subMenus),
                                                                                             size(subMenusSize)
    {}

    MenuWithOptions(const String &name, void (*func)()) : name(name), func(func)
    {}

    MenuWithOptions()
    {}

private:
    String name;
    MenuWithOptions *children;
    int size;
    int currentIndex{};

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
        setCurrentIndex(currentIndex + children[currentIndex].nextIndex());
        Serial.println(name);
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
        setCurrentIndex(currentIndex + children[currentIndex].previousIndex());
        Serial.println(name);
        return 0;
    }

    /// enter sub menu - Changes display (title and sub options) or executes the chosen option. Recursively calls enter sub menu, until called menu doesn't have children, execute() is called.
    String *enterSubMenu()
    {
        // If menu doesn't have children, and enter sub menu was called on it, that means it's the last in the chain, and should execute its method
        // Returns null pointer so parent can properly return getNames()
        if (!hasChildren())
        {
            func();
            return nullptr;
        }

        MenuWithOptions currentChild = children[currentIndex];
        // If menu has opened child, call enter sub menu on the child.
        // If the child returned null, meaning it doesn't have children (so doesn't have names to display), display names
        if (hasSubMenuOpened)
        {
            String *names = currentChild.enterSubMenu();
            if (names == nullptr)
            {
                return getNames();
            }
            return names;
        }
        // If menu doesn't have opened children, but has children, open sub menu (change the bool) and display names
        hasSubMenuOpened = true;
        return getNames();
    }

    String *exitSubMenu()
    {
        // Can do that without checking whether menu has children. Method will never get called for menu not having children
        MenuWithOptions currentChild = children[currentIndex];
        if (hasSubMenuOpened)
        {
            String *childName = currentChild.exitSubMenu();
            if (childName == nullptr)
            {
                return getNames();
            }

            return childName;
        }

        hasSubMenuOpened = false;
        return nullptr;
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
    if(buttonPressedThisFrame(BUTTON_GREEN))
    {
        value++;
        if(value > 255) value -= 255;
    }
    ledPower[0] = value;
    analogWrite(LED_RED, value);
}
void GreenLed()
{
    int value = ledPower[1];
    if(buttonPressedThisFrame(BUTTON_GREEN))
    {
        value++;
        if(value > 255) value -= 255;
    }
    ledPower[1] = value;
    analogWrite(LED_GREEN, value);
}
void BlueLed()
{
    int value = ledPower[2];
    if(buttonPressedThisFrame(BUTTON_GREEN))
    {
        value++;
        if(value > 255) value -= 255;
    }
    ledPower[2] = value;
    analogWrite(LED_BLUE, value);
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
    if(currentTemperatureFormat == celcius) return temperature;
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
                                                                       {">",             empty},
                                                                       {"-",             empty},
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
                                       {"About",       empty}
                               });
    mainMenu.enterSubMenu();
}


void setup()
{
//    initLCD();
    initButtons();
    initRGB();
    initEncoder();
    initLED();
    Serial.begin(9600);
    setupPCI();
    initMainMenu();

//    mainMenu = MenuWithOptions("Main Menu", 4,
//                               (MenuWithOptions[])
//                                       {{"LED Options", 4,
//                                                (MenuWithOptions[]) {
//                                                        {"Power", 2, (MenuWithOptions[]) {{"On",  ledOn},
//                                                                                          {"Off", ledOff}}},
//                                                        {"Red",   empty},
//                                                        {"Green", empty},
//                                                        {"Blue",  empty}
//                                                }},
//                                        {"Display",     2,
//                                                (MenuWithOptions[]) {
//                                                        {"Backlight", 2, (MenuWithOptions[]) {{"On",  empty},
//                                                                                              {"Off", empty}}},
//                                                        {"Selector",  3, (MenuWithOptions[]) {{">",             empty},
//                                                                                              {"-",             empty},
//                                                                                              {"<custom char>", empty}}}
//                                                }},
//                                        {"Temperature", 3,
//                                                (MenuWithOptions[]) {
//                                                        {"Sensor IN",  empty},
//                                                        {"Sensor OUT", empty},
//                                                        {"Units",      2, (MenuWithOptions[]) {{"C", empty},
//                                                                                               {"F", empty}}}
//                                                }},
//                                        {"About",       empty}});
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
    if (buttonPressed(BUTTON_GREEN))
    {
        mainMenu.enterSubMenu();
    } else if (buttonPressed(BUTTON_RED))
    {
        mainMenu.exitSubMenu();
    }
}