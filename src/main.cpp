#include <Arduino.h>
#include <EepromAnything.h>
#include <LiquidCrystal_I2C.h>

#define LCD_I2C_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4
#define LCD_REFRESH_INTERVAL 300
#define PREGAME_COUNTER 4

/* #region State Machine */
enum class GameState { None, Pregame, Running, End };

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
unsigned long gameStartedAt, lastHitAt, pressReactTime, oldPressReactTime,
    bestScore, oldBestScore = 0;

uint8_t session, oldSession = 1;

unsigned long userShouldPressAt = 0;
bool userShouldPress = false;

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

void setup() {
    Serial.begin(115200);

    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    showLed(0, 0, 0);

    pinMode(buttonPin, INPUT);

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);

    setupTimer1();
    setupTimer2();

    watermark();

    screenNeedsUpdate = true;
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
}

void checkState() {
    switch (State) {
        case GameState::None: {
            if (screenNeedsUpdate) {
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

void checkInput() {
    switch (State) {
        case GameState::None: {
            uint8_t buttonState = digitalRead(buttonPin);

            if (buttonState == HIGH) startCountdown();

            break;
        }
        case GameState::Running: {
            uint8_t buttonState = digitalRead(buttonPin);

            // supposed to press and never pressed
            if (buttonState == HIGH && hasHit == false &&
                userShouldPress == true) {
                showLed(0, 255, 0);
                hasHit = true;
                lastHitAt = millis();

                pressReactTime =
                    lastHitAt - (gameStartedAt + userShouldPressAt);

                if (pressReactTime < bestScore || bestScore == 0)
                    bestScore = pressReactTime;
            }

            // on release after a successfull press
            if (buttonState == LOW && hasHit == true) {
                if (session == 10) {  // finish game

                } else {
                    hasHit = false;
                    gameTime = 0;
                    gameStartedAt = millis();
                    userShouldPress = false;
                    userShouldPressAt = random(3, 5) * random(1000, 2000);
                    session++;
                }
            }

            break;
        }
    }
}

unsigned long lastBlinkedAt = 0;
uint8_t currentRed = 0;

void checkLed() {
    if (!userShouldPress) {
        if (millis() - lastBlinkedAt > 100) {
            lastBlinkedAt = millis();

            if (currentRed == 255)
                currentRed = 0;
            else
                currentRed = 255;

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

void showScores() {
    char lines[LCD_ROWS][LCD_COLS + 1] = {{"   -SKOR TABLOSU-   "},
                                          {"1: 0000 ms          "},
                                          {"2: 0000 ms          "},
                                          {"3: 0000 ms          "}};

    /*sprintf(lines[1], "TIME:       00:%02d:%02d", bombCountdownTime / 60,
            bombCountdownTime % 60);
    sprintf(lines[2], "CODE:        %s", bombPassword);*/

    printWithAnimation(lines);
}

void showPregame() {
    char lines[LCD_ROWS][LCD_COLS + 1] = {{"   - HAZIRLANIN -   "},
                                          {"Led kirmizi olunca  "},
                                          {"basin, mavi olunca  "},
                                          {"birakin          5sn"}};

    printWithAnimation(lines);
}

void showGame() {
    char lines[LCD_ROWS][LCD_COLS + 1] = {{"Sure: 00:00.000     "},
                                          {"Basma tepki: 0ms    "},
                                          {"En iyi skor: 0ms    "},
                                          {"Deneme: 1/10        "}};

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

void startCountdown() {
    State = GameState::Pregame;
    screenNeedsUpdate = true;
    pregameCounter = PREGAME_COUNTER;
}

void startGame() {
    State = GameState::Running;
    screenNeedsUpdate = true;

    showLed(255, 0, 0);
    gameStartedAt = millis();
    userShouldPressAt = random(3, 5) * random(1000, 2000);

    Serial.print("Should Press At: ");
    Serial.println(userShouldPressAt);
    session = 1;
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
                lcd.print("/10");
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
}

ISR(TIMER2_COMPA_vect) {
    if (State == GameState::Running) {
        gameTime += 2;
    }
}