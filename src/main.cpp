#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

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

  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:

  
}