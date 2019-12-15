#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <EepromAnything.h>

#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4
#define LCD_REFRESH_INTERVAL 300
#define PREGAME_COUNTER 9

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

const uint8_t redPin = 6;
const uint8_t greenPin = 5;
const uint8_t bluePin = 3;

const uint8_t buttonPin = 8;

GameState State;
bool screenNeedsUpdate = false;
uint8_t pregameCounter, oldPregameCounter = 0;

unsigned long gameTime, oldGameTime = 0;

void watermark();
void checkState();
void checkInput();
void showScores();
void showPregame();
void showGame();
void startCountdown();
void startGame();
void printWithAnimation(char bucket[LCD_ROWS][LCD_COLS + 1]);
void setupTimer1();
void setupTimer2();
void partialUpdates();

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

  setupTimer1();
  setupTimer2();

  watermark();

  screenNeedsUpdate = true;
}

void loop()
{
  checkState();
  checkInput();
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

void checkState()
{
  switch (State)
  {
    case GameState::None:
    {
      if (screenNeedsUpdate)
      {
        showScores();
        screenNeedsUpdate = false;
      }

      break;
    }
    case GameState::Pregame:
    {
      if (screenNeedsUpdate)
      {
        showPregame();
        screenNeedsUpdate = false;
      }

      break;
    }

    case GameState::Running: {
      if (screenNeedsUpdate) {
        showGame();
        screenNeedsUpdate = false;
      }

      break;
    }
  }

  if (pregameCounter != oldPregameCounter) {
    oldPregameCounter = pregameCounter;

    partialUpdates();
  }
  else if (gameTime - oldGameTime > LCD_REFRESH_INTERVAL) {
    oldGameTime = gameTime;
    
    partialUpdates();
  }
}

void checkInput()
{
  switch (State)
  {
    case GameState::None:
    {
      uint8_t buttonState = digitalRead(buttonPin);

      if (buttonState == LOW)
        startCountdown();

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

void showPregame()
{
  char lines[LCD_ROWS][LCD_COLS + 1] = {{"   - HAZIRLANIN -   "},
                                        {"Led kirmizi olunca  "},
                                        {"birakin, yesil olun-"},
                                        {"ca basin.        5sn"}};

  printWithAnimation(lines);
}

void showGame()
{
  char lines[LCD_ROWS][LCD_COLS + 1] = {{"Sure: 00:00.000     "},
                                        {"Basma Tepki: 23ms   "},
                                        {"Birakma Tep: 23ms   "},
                                        {"Deneme: 1/10        "}};

  printWithAnimation(lines);
}

void printWithAnimation(char bucket[LCD_ROWS][LCD_COLS + 1])
{
  lcd.clear();

  for (uint8_t row = 0; row < LCD_ROWS; row++)
  {
    for (uint8_t col = 0; col < LCD_COLS; col++)
    {
      lcd.setCursor(col, row);
      lcd.print(bucket[row][col]);
      delay(1);
    }
  }
}

void startCountdown()
{
  State = GameState::Pregame;
  screenNeedsUpdate = true;
  pregameCounter = PREGAME_COUNTER;
}

void startGame()
{
  State = GameState::Running;
  screenNeedsUpdate = true;
}

void partialUpdates()
{
  if (screenNeedsUpdate == false)
  {
    if (State == GameState::Pregame)
    {
      lcd.setCursor(17, 3);
      lcd.print(pregameCounter);
      lcd.print("sn");
    }
    else if (State == GameState::Running)
    {
      lcd.setCursor(6, 0);
      char text[21];
      uint16_t min = gameTime / 1000 / 60;
      uint16_t sec = (gameTime / 1000) % 60;
      uint16_t mil = gameTime % 1000;

      sprintf(text, "%02d:%02d.%03d", min, sec, mil);

      lcd.print(text);
    }
  }
}

void setupTimer1()
{
  noInterrupts();
  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // 1 Hz (16000000/((15624+1)*1024))
  OCR1A = 15624;
  // CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // Output Compare Match A Interrupt Enable
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

void setupTimer2()
{
  noInterrupts();
  // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;

  // 500 Hz (16000000/((124+1)*256))
  OCR2A = 124;
  // CTC
  TCCR2A |= (1 << WGM21);
  // Prescaler 256
  TCCR2B |= (1 << CS22) | (1 << CS21);
  // Output Compare Match A Interrupt Enable
  TIMSK2 |= (1 << OCIE2A);
  interrupts();
}

ISR(TIMER1_COMPA_vect)
{
  if (State == GameState::Pregame)
  {
    if (pregameCounter > 0)
      pregameCounter--;

    if (pregameCounter == 0)
      startGame();
  }
}

ISR(TIMER2_COMPA_vect)
{
  if (State == GameState::Running) {
    gameTime += 2;
  }
}