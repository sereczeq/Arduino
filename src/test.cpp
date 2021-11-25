//
// Created by kubas on 10.11.2021.
//


#include <LiquidCrystal_I2C.h>
#include "DallasTemperature.h"
#include "OneWire.h"


LiquidCrystal_I2C lcd(0x27, 16, 2);
#define ONE_WIRE_BUS A1
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress outsideSensorAddress;
DeviceAddress insideSensorAddress;

float outside_temperature = 0;

float inside_temperature = 0;


void printSensorAddresses()
{
    delay(100);

    oneWire.reset_search(); //reset search state

    sensors.getAddress(outsideSensorAddress, 0);
    sensors.getAddress(insideSensorAddress, 1);
}

void initLcd()
{
    lcd.init();
    lcd.clear();
    lcd.backlight();
    lcd.clear();
    lcd.home();
}

void drawLcd()
{
    lcd.setCursor(0, 0);

    lcd.print("O: " + String(outside_temperature, 1));

    lcd.setCursor(0, 1);

    lcd.print("I: " + String(inside_temperature, 1));
}

void setup(void)
{
    Serial.begin(9600);
    sensors.begin();

    printSensorAddresses();
    initLcd();

    drawLcd();
}

void loop(void)
{
    sensors.requestTemperatures();

    outside_temperature = sensors.getTempC(outsideSensorAddress);
    inside_temperature = sensors.getTempC(insideSensorAddress);

    drawLcd();
    delay(1000);
}