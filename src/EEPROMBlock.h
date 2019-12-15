#pragma once
#ifndef EEPROMBlock_h
    #define EEPROMBlock_h
    
#include <Arduino.h>

struct config_t {
  char Version[4];
  uint16_t DominationGameDuration; //  When game mode is 0 = Any of the teams reaches this much domination duration, is elected as the winner, 0-1800
  //When game mode = 1, After this much time has elapsed, game ends and winner is selected based on who is longer dominated, in seconds 0-1800

  uint8_t DominationGameMode; // 0 = Fixed Domination Period,  1 = Fixed Game Duration
  uint8_t DominationRfidDuration; // The duration needed for rfid card to be present on the reader, during domination or neutralization period, in seconds 0-255

  char BombPassword[8]; // The seven-digit password for the bomb, must be seven digit
  uint16_t BombGameDuration; // Countdown timer for the bomb game, in seconds 0-1800
  uint8_t BombExplodesOnWrongEntry; // Should bomb explode when someone enters wrong code? // 0= No, 1= Input is checked after all 7 digits are supplied, 2= Every key press is checked, any wrong key = boom
  uint8_t BombRfidDefuseDuration; // The duration needed for rfid card to be present on the reader, during defusing period of the bomb, in seconds 0-255

  uint8_t DominationStartUpDelay;
  uint16_t DominationPressureDuration;
  uint16_t NeutralizationPressureDuration; 
};
#endif