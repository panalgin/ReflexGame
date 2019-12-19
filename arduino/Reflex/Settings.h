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

    uint16_t PropertyLimits[10][2] = {};

    uint8_t Load(void);
    uint8_t Save(void);
    char *GetValue(uint8_t index);
    bool SetValue(uint8_t index, char *value);
    void SetValueRaw(uint8_t index, char *value);
    char GetLastError();
};

#endif
