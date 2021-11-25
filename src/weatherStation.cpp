#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

#define ONE_WIRE_BUS A1

#define RED_BUTTON_PIN 2
#define RED 6
#define BLUE 3

LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress outside_sensor_address;
DeviceAddress inside_sensor_address;

float outside_temperature = 0;
float outside_temperature_min = 1000;
float outside_temperature_max = -1000;

float inside_temperature = 0;
float inside_temperature_min = 1000;
float inside_temperature_max = -1000;

int redButtonState = HIGH;
int lastRedButtonState = HIGH;
unsigned long lastRedDebounceTime = 0;
unsigned long debounceDelay = 50;

bool red_state = LOW;
bool blue_state = LOW;

void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        Serial.print("0x");
        if (deviceAddress[i] < 0x10) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
        if (i < 7) Serial.print(", ");
    }
    Serial.println("");
}

void printSensorAddresses() {
    delay(100);

    oneWire.reset_search(); //reset search state

    Serial.println("Locating devices...");
    Serial.print("Found ");
    int deviceCount = sensors.getDeviceCount();
    Serial.print(deviceCount, DEC);
    Serial.println(" devices.");
    Serial.println("");

    Serial.println("Printing addresses...");
    for (int i = 0;  i < deviceCount;  i++)
    {
        if(i == 0) {
            Serial.print("Sensor ");
            Serial.print(i+1);
            Serial.print(" : ");
            sensors.getAddress(outside_sensor_address, i);
            printAddress(outside_sensor_address);
        } else if (i == 1) {
            Serial.print("Sensor ");
            Serial.print(i+1);
            Serial.print(" : ");
            sensors.getAddress(inside_sensor_address, i);
            printAddress(inside_sensor_address);
        }

    }
}

void initLcd() {
    lcd.init();
    lcd.clear();
    lcd.backlight();
    lcd.clear();
    lcd.home();
}

void drawLcd() {
    lcd.setCursor(0, 0);

    lcd.print("O" + String(outside_temperature, 1));
    lcd.setCursor(5, 0);
    lcd.print((char)223);
    lcd.setCursor(6, 0);
    lcd.print("-" + String(outside_temperature_min, 1));
    lcd.setCursor(11, 0);
    lcd.print("+" + String(outside_temperature_max, 1));

    lcd.setCursor(0, 1);

    lcd.print("I" + String(inside_temperature, 1));
    lcd.setCursor(5, 1);
    lcd.print((char)223);
    lcd.setCursor(6, 1);
    lcd.print("-" + String(inside_temperature_min, 1));
    lcd.setCursor(11, 1);
    lcd.print("+" + String(inside_temperature_max, 1));
}

void setup(void)
{
    Serial.begin(9600);
    sensors.begin();

    pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RED, OUTPUT);
    pinMode(BLUE, OUTPUT);

    printSensorAddresses();
    initLcd();

    drawLcd();
}

void calculateMinMax() {
    if(outside_temperature > outside_temperature_max) {
        outside_temperature_max = outside_temperature;
    }
    if(outside_temperature < outside_temperature_min) {
        outside_temperature_min = outside_temperature;
    }

    if(inside_temperature > inside_temperature_max) {
        inside_temperature_max = inside_temperature;
    }
    if(inside_temperature < inside_temperature_min) {
        inside_temperature_min = inside_temperature;
    }

    if(outside_temperature > 25) {
        red_state = HIGH;
        blue_state = LOW;
    } else{
        red_state = LOW;
        blue_state = HIGH;
    }


}

unsigned long lastFetchDebounceTime = 0;

void loop(void)
{
    digitalWrite(RED, red_state);
    digitalWrite(BLUE, blue_state);

    int redReading = digitalRead(RED_BUTTON_PIN);

    if (redReading != lastRedButtonState)
    {
        lastRedDebounceTime = millis();
    }

    if ((millis() - lastRedDebounceTime) > debounceDelay)
    {
        if (redReading != redButtonState)
        {
            redButtonState = redReading;
        }
    }

    if(redReading != lastRedButtonState) {
        if(redReading == HIGH) {
            outside_temperature_min = 1000;
            outside_temperature_max = -1000;

            inside_temperature_min = 1000;
            inside_temperature_max = -1000;
        }
    }

    lastRedButtonState = redReading;

    if ((millis() - lastFetchDebounceTime) > 1000) {
        lastFetchDebounceTime = millis();


        sensors.requestTemperatures();

        outside_temperature = sensors.getTempC(outside_sensor_address);
        inside_temperature = sensors.getTempC(inside_sensor_address);

        calculateMinMax();
        drawLcd();
    }


}
