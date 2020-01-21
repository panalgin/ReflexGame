#include <Arduino.h>
#include <EepromAnything.h>
#include <LiquidCrystal_I2C.h>
#include "Settings.h"
#include "TonePlayer.h"

#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4
#define LCD_REFRESH_INTERVAL 300
#define PREGAME_COUNTER 9
#define SESSION_RETRIES 5

/* #region State Machine */
enum class GameState { None, Pregame, Running, End };

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

char brandFirst[LCD_COLS + 1]   = "    REFLEX OYUNU    ";
char brandSecond[LCD_COLS + 1]  = "        v2.0        ";
char brandThird[LCD_COLS + 1]   = "   MUSTAFA CANSIZ   ";
char brandFourth[LCD_COLS + 1]  = "--------------------";
char brandFifth[LCD_COLS + 1]   = "    METIN YALCIN    ";

const uint8_t redPin = 6;
const uint8_t greenPin = 5;
const uint8_t bluePin = 3;

const uint8_t buzzerPin = 9;
const uint8_t buttonPin = 8;

GameState State;
Settings settings;

bool screenNeedsUpdate = false;
uint8_t pregameCounter, oldPregameCounter = 0;

unsigned long gameTime, oldGameTime = 0;
unsigned long gameStartedAt, lastHitAt, pressReactTime, oldPressReactTime,
    bestScore, oldBestScore = 0;

uint8_t session, oldSession = 1;

unsigned long userShouldPressAt = 0;
unsigned long totalScore = 0;
bool userShouldPress = false;

volatile uint8_t nextPageTimer = 0;
TonePlayer tone1(TCCR1A, TCCR1B, OCR1AH, OCR1AL, TCNT1H,
                 TCNT1L);  // Uno 9th pin

void watermark();
void checkState();
void checkInput();
void checkLed();
void showScores();
void showPregame();
void showGame();
void startCountdown();
void startGame();
void printWithAnimation(char bucket[LCD_ROWS][LCD_COLS + 1]);
void setupTimer1();
void setupTimer2();
void partialUpdates();
void showLed(uint8_t red, uint8_t green, uint8_t blue);
void endGame();
void showEnd();
void resetGame();
void celebrate();

void setup() {
    Serial.begin(115200);

    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    showLed(0, 0, 0);

    pinMode(buttonPin, INPUT);
    pinMode(buzzerPin, OUTPUT);

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);

    watermark();

    screenNeedsUpdate = true;
    settings.Load();

    setupTimer1();
    setupTimer2();
}

void loop() {
    checkState();
    checkInput();
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
    lcd.clear();
    lcd.home();

    for (uint8_t i = 0; i < LCD_COLS; i++) {
        lcd.setCursor(i, 1);
        lcd.write(brandThird[i]);

        if (brandThird[i] != ' ') delay(75);
    }

    for (uint8_t i = 0; i < LCD_COLS; i++) {
        lcd.setCursor(i, 2);
        lcd.write(brandFourth[i]);

        if (brandFourth[i] != ' ') delay(75);
    }

    for (uint8_t i = 0; i < LCD_COLS; i++) {
        lcd.setCursor(i, 3);
        lcd.write(brandFifth[i]);

        if (brandFifth[i] != ' ') delay(75);
    }

    delay(3000);
}

void checkState() {
    switch (State) {
        case GameState::None: {
            if (screenNeedsUpdate || nextPageTimer > 5) {
                nextPageTimer = 0;
                showScores();
                screenNeedsUpdate = false;
            }

            break;
        }
        case GameState::Pregame: {
            if (screenNeedsUpdate) {
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

            checkLed();

            break;
        }
        case GameState::End: {
            if (screenNeedsUpdate) {
                showEnd();
                screenNeedsUpdate = false;
            }

            break;
        }
    }

    if (pregameCounter != oldPregameCounter) {
        oldPregameCounter = pregameCounter;

        partialUpdates();
    } else if (gameTime - oldGameTime > LCD_REFRESH_INTERVAL) {
        oldGameTime = gameTime;

        partialUpdates();
    }
}

bool hasHit = false;
bool canHit = true;

void checkInput() {
    switch (State) {
        case GameState::None: {
            uint8_t buttonState = digitalRead(buttonPin);

            if (buttonState == HIGH) startCountdown();

            break;
        }
        case GameState::Pregame:
            break;
        case GameState::Running: {
            uint8_t buttonState = digitalRead(buttonPin);

            // supposed to press and never pressed
            if (canHit == true && buttonState == HIGH && hasHit == false) {
                canHit = false;  // can't rehit before releasing

                if (userShouldPress) {
                    showLed(0, 255, 0);
                    hasHit = true;
                    lastHitAt = millis();

                    pressReactTime =
                        lastHitAt - (gameStartedAt + userShouldPressAt);

                    if (pressReactTime < bestScore || bestScore == 0)
                        bestScore = pressReactTime;

                    totalScore += pressReactTime;
                } else {  // user might be pressing randomly, so give penalty to
                          // prevent accidental quick reaction
                    userShouldPressAt += 1000;
                }
            }

            // on release after a successfull press
            if (buttonState == LOW) {
                canHit = true;  // can re-hit after this

                if (hasHit) {  // was a successfull button press
                    if (session == SESSION_RETRIES) {  // finish game
                        endGame();
                    } else {
                        hasHit = false;
                        gameTime = 0;
                        gameStartedAt = millis();
                        userShouldPress = false;
                        userShouldPressAt = random(2, 5) * random(1000, 2000);
                        session++;
                    }
                }
            }

            break;
        }
        case GameState::End: {
            uint8_t buttonState = digitalRead(buttonPin);

            if (buttonState == HIGH) resetGame();

            break;
        }
    }
}

unsigned long lastBlinkedAt = 0;
uint8_t currentRed = 0;

void checkLed() {
    if (!userShouldPress) {
        if (millis() - lastBlinkedAt > 50) {
            lastBlinkedAt = millis();

            if (currentRed == 100)
                currentRed = 0;
            else
                currentRed = 100;

            showLed(currentRed, 0, 0);
        }
    }

    if (userShouldPressAt < gameTime && userShouldPress == false) {
        userShouldPress = true;
        showLed(0, 0, 255);
    }
    // check if button is pressed, record start - button press

    // if releases button, fails
    // after a random time, stop led, record time
    // c
}

bool isFirstPage = true;

void showScores() {
    char lines[LCD_ROWS][LCD_COLS + 1] = {{"   -SKOR TABLOSU-   "},
                                          {"1:                  "},
                                          {"2:                  "},
                                          {"3:                  "}};

    if (isFirstPage) {
        uint16_t first = settings.EepromBlock.Scores[0];
        uint16_t second = settings.EepromBlock.Scores[1];
        uint16_t third = settings.EepromBlock.Scores[2];

        if (first != UINT16_MAX)
            sprintf(lines[1], "1: %u ms", (uint16_t)first);
        else
            sprintf(lines[1], "1: %s", "---");

        if (second != UINT16_MAX)
            sprintf(lines[2], "2: %u ms", (uint16_t)second);
        else
            sprintf(lines[2], "2: %s", "---");

        if (third != UINT16_MAX)
            sprintf(lines[3], "3: %u ms", (uint16_t)third);
        else
            sprintf(lines[3], "3: %s", "---");

        isFirstPage = false;
    } else {
        uint16_t fourth = settings.EepromBlock.Scores[3];
        uint16_t fifth = settings.EepromBlock.Scores[4];

        if (fourth != UINT16_MAX)
            sprintf(lines[1], "4: %u ms", (uint16_t)fourth);
        else
            sprintf(lines[1], "4: %s", "---");

        if (fifth != UINT16_MAX)
            sprintf(lines[2], "5: %u ms", (uint16_t)fifth);
        else
            sprintf(lines[2], "5: %s", "---");

        sprintf(lines[3], "%s", "                    ");

        isFirstPage = true;
    }

    printWithAnimation(lines);
}

void showPregame() {
    char lines[LCD_ROWS][LCD_COLS + 1] = {{"   - HAZIRLANIN -   "},
                                          {"Led mavi renk oldu -"},
                                          {"gunda, hemen basin  "},
                                          {"Baslamaya:       5sn"}};

    printWithAnimation(lines);
}

void showGame() {
    char lines[LCD_ROWS][LCD_COLS + 1] = {{"Sure: 00:00.000     "},
                                          {"Basma tepki: 0ms    "},
                                          {"En iyi skor: 0ms    "},
                                          {"Deneme: 1/5         "}};

    printWithAnimation(lines);
}

void showEnd() {
    char lines[LCD_ROWS][LCD_COLS + 1] = {{"---  TEBRIKLER   ---"},
                                          {"En iyi skor: 0ms    "},
                                          {"Ortalamaniz: 0ms    "},
                                          {"Skorunuz kaydedildi "}};

    uint16_t avgScore = (uint16_t)(totalScore / SESSION_RETRIES);

    sprintf(lines[1], "En iyi skor: %ums", (uint16_t)bestScore);
    sprintf(lines[2], "Ortalamaniz: %ums", (uint16_t)avgScore);

    printWithAnimation(lines);

    settings.Assert(avgScore);
    settings.Save();

    bool isInScoreboard = avgScore < settings.EepromBlock.Scores[2];

    if (isInScoreboard) {
        Serial.println("Need to celebrate");
        celebrate();
        // play buzzer
        // animate
        // change screen
    }
}

void celebrate() {
    unsigned long startedAt = millis();
    unsigned long elapsed = 0;
    uint8_t buzzerState = 1;

    while (millis() - startedAt < 5000) {
        elapsed = millis() - startedAt;

        if (elapsed % 200 == 0) {
            if (buzzerState == HIGH) {
                tone1.tone(1000);
                delay(100);
            } else {
                tone1.tone(500);
                delay(100);
            }

            buzzerState = !buzzerState;
        }

        if (elapsed % 30 == 0) {
            uint8_t red = random(0, 255);
            uint8_t green = random(0, 255);
            uint8_t blue = random(0, 128);

            showLed(red, green, blue);
        }
    }

    uint16_t fadeFrom = 500;

    while (fadeFrom > 0) {
        tone1.tone(fadeFrom);
        delay(10);

        fadeFrom -= 10;
    }

    tone1.noTone();
}

void printWithAnimation(char bucket[LCD_ROWS][LCD_COLS + 1]) {
    lcd.clear();

    for (uint8_t row = 0; row < LCD_ROWS; row++) {
        for (uint8_t col = 0; col < LCD_COLS; col++) {
            lcd.setCursor(col, row);
            char c = bucket[row][col];

            if (c != '\0')
                lcd.print(bucket[row][col]);
            else
                lcd.print(' ');

            delayMicroseconds(500);
        }
    }
}

void startCountdown() {
    State = GameState::Pregame;
    screenNeedsUpdate = true;
    pregameCounter = PREGAME_COUNTER;
}

void startGame() {
    State = GameState::Running;
    screenNeedsUpdate = true;

    showLed(255, 0, 0);
    session = 1;

    delay(500);
    gameStartedAt = millis();
    userShouldPressAt = random(2, 5) * random(1000, 2000);
}

void endGame() {
    State = GameState::End;
    screenNeedsUpdate = true;
    showLed(255, 255, 0);
    gameTime = 0;
}

void resetGame() {
    State = GameState::None;
    screenNeedsUpdate = true;
    showLed(0, 0, 0);
    bestScore = 0;
    totalScore = 0;
    canHit = true;
    userShouldPress = false;
    hasHit = false;
    gameTime = 0;
    session = 0;

    setupTimer1();
    setupTimer2();
}

void partialUpdates() {
    if (screenNeedsUpdate == false) {
        if (State == GameState::Pregame) {
            lcd.setCursor(17, 3);
            lcd.print(pregameCounter);
            lcd.print("sn");
        } else if (State == GameState::Running) {
            lcd.setCursor(6, 0);
            char text[21];
            uint16_t min = gameTime / 1000 / 60;
            uint16_t sec = (gameTime / 1000) % 60;
            uint16_t mil = gameTime % 1000;

            sprintf(text, "%02d:%02d.%03d", min, sec, mil);

            lcd.print(text);

            if (pressReactTime != oldPressReactTime) {
                oldPressReactTime = pressReactTime;

                lcd.setCursor(13, 1);
                lcd.print("       ");
                lcd.setCursor(13, 1);
                lcd.print(pressReactTime);
                lcd.print("ms");
            }

            if (bestScore != oldBestScore) {
                oldBestScore = bestScore;
                lcd.setCursor(13, 2);
                lcd.print("       ");
                lcd.setCursor(13, 2);
                lcd.print(bestScore);
                lcd.print("ms");
            }

            if (session != oldSession) {
                oldSession = session;
                lcd.setCursor(8, 3);
                lcd.print(session);
                lcd.print("/5");
            }
        }
    }
}

void showLed(uint8_t red, uint8_t green, uint8_t blue) {
    analogWrite(redPin, 255 - red);
    analogWrite(greenPin, 255 - green);
    analogWrite(bluePin, 255 - blue);
}

void setupTimer1() {
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

void setupTimer2() {
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

ISR(TIMER1_COMPA_vect) {
    if (State == GameState::Pregame) {
        if (pregameCounter > 0) pregameCounter--;
        if (pregameCounter == 0) startGame();
    }

    if (State == GameState::None) {
        nextPageTimer++;
    }
}

ISR(TIMER2_COMPA_vect) {
    if (State == GameState::Running) {
        gameTime += 2;
    }
}