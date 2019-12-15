#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <EepromAnything.h>

#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

char brandFirst[LCD_COLS + 1]  = "    REFLEX GAME     ";
char brandSecond[LCD_COLS + 1] = "      v: 0.9b       ";

const uint8_t redPin = 10;
const uint8_t greenPin = 11;
const uint8_t bluePin = 12;

const uint8_t buttonPin = 5;

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);

  watermark();
}

void watermark() {
  lcd.clear();
  lcd.home();

  for (uint8_t i = 0; i < LCD_COLS; i++) {
    lcd.setCursor(i, 1);
    lcd.write(brandFirst[i]);

    if (brandFirst[i] != ' ') delay(75);
  }

  for (uint8_t i = 0; i < LCD_COLS; i++) {
    lcd.setCursor(i, 2);
    lcd.write(brandSecond[i]);

    if (brandSecond[i] != ' ') delay(75);
  }

  delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:


}