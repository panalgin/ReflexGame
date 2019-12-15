#ifndef Settings_h
#define Settings_h

#include <Arduino.h>
#include "EEPROMBlock.h"

#define _DEBUG

class Settings
{
private:
    uint8_t ErrorCodes[8] = {
        0, // OK
        1, // Input value was bigger than the maximum allowed
        2, // Input value was smaller than the minimum allowed
    };

    uint8_t LastErrorCode;

public:
    Settings();
    config_t EepromBlock;
    bool IsModified;

    uint16_t PropertyLimits[10][2] = {
        {5, 43200}, // DominationGameDuration Min-Max  // Max 12 Hours
        {0, 1},     // DominationGameMode Min-Max
        {1, 10},    // DominationRfidDuration Min-Max
        {7, 7},     // BombPassword Length Min-Max
        {5, 1800},  // BombGameDuration Min-Max
        {0, 2},     // BombExplodesOnWrongEntry 0: No, 1: Yes On Complete, 2: Yes On Each Key
        {1, 10},    // BombRfidDefuseDuration Min-Max
        {0, 255},   // Domination StartUp Delay Min-Max
        {1, 600},   // Domination Pressure Duration, Min-Max
        {1, 600},   // Neutralization Pressure Duration, Min-Max
    };

    uint8_t Load(void);
    uint8_t Save(void);
    char *GetValue(uint8_t index);
    bool SetValue(uint8_t index, char *value);
    void SetValueRaw(uint8_t index, char *value);
    char GetLastError();
};

#endif