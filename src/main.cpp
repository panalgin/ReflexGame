#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <EepromAnything.h>

#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4

/* #region State Machine */
enum class GameState
{
  None,
  Pregame,
  Running
};

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

char brandFirst[LCD_COLS + 1] = "    REFLEX OYUNU    ";
char brandSecond[LCD_COLS + 1] = "      v: 0.92b      ";

const uint8_t redPin = 3;
const uint8_t greenPin = 4;
const uint8_t bluePin = 5;

const uint8_t buttonPin = 5;

GameState State;
bool screenNeedsUpdate = false;

void watermark();
void checkState();
void showScores();
void printWithAnimation(char bucket[LCD_ROWS][LCD_COLS + 1]);

void setup()
{
  Serial.begin(115200);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);

  watermark();

  screenNeedsUpdate = true;
}

void watermark()
{
  lcd.clear();
  lcd.home();

  for (uint8_t i = 0; i < LCD_COLS; i++)
  {
    lcd.setCursor(i, 1);
    lcd.write(brandFirst[i]);

    if (brandFirst[i] != ' ')
      delay(75);
  }

  for (uint8_t i = 0; i < LCD_COLS; i++)
  {
    lcd.setCursor(i, 2);
    lcd.write(brandSecond[i]);

    if (brandSecond[i] != ' ')
      delay(75);
  }

  delay(2000);
}

void loop()
{
  // put your main code here, to run repeatedly:

  checkState();
}

void checkState()
{
  switch (State)
  {
    case GameState::None:
    {
      if (screenNeedsUpdate) {
        showScores();
        screenNeedsUpdate = false;
      }

      break;
    }
  }
}

void showScores()
{
  char lines[LCD_ROWS][LCD_COLS + 1] = {{"   -SKOR TABLOSU-   "},
                                        {"1: 0023 ms          "},
                                        {"2: 0035 ms          "},
                                        {"3: 0250 ms          "}};

  /*sprintf(lines[1], "TIME:       00:%02d:%02d", bombCountdownTime / 60,
          bombCountdownTime % 60);
  sprintf(lines[2], "CODE:        %s", bombPassword);*/


  printWithAnimation(lines);
}

void printWithAnimation(char bucket[LCD_ROWS][LCD_COLS + 1]) {
  lcd.clear();

  for (uint8_t row = 0; row < LCD_ROWS; row++) {
    for (uint8_t col = 0; col < LCD_COLS; col++) {
      lcd.setCursor(col, row);
      lcd.print(bucket[row][col]);
      delay(1);
    }
  }
}