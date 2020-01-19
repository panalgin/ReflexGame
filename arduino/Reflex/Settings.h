#ifndef Settings_h
#define Settings_h

#include <Arduino.h>
#include "EEPROMBlock.h"

#define _DEBUG

class Settings {
   private:
    uint8_t ErrorCodes[8] = {
        0,  // OK
        1,  // Input value was bigger than the maximum allowed
        2,  // Input value was smaller than the minimum allowed
    };

    uint8_t LastErrorCode;

   public:
    Settings();
    config_t EepromBlock;
    bool IsModified;

    uint8_t Load(void);
    uint8_t Save(void);
    void Assert(uint16_t avgScore);
};

#endif