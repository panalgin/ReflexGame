#include "Settings.h"
#include "EEPROMAnything.h"

/// Base constructor
Settings::Settings() {}

/*
 * Reads and loads setting values from the eeprom
 */
uint8_t Settings::Load(void) {
    EEPROM_readAnything(0, EepromBlock);

    if (strcmp(this->EepromBlock.Version, "123") != 0) {
        strcpy(this->EepromBlock.Version, "123");
        this->EepromBlock.Scores[0] = 0;
        this->EepromBlock.Scores[1] = 0;
        this->EepromBlock.Scores[2] = 0;

        EEPROM_writeAnything(0, this->EepromBlock);
    }

#ifdef _DEBUG
    // Print eeprom values
#endif

    return 0;
}

uint8_t Settings::Save(void) {
    if (this->IsModified) {
        this->IsModified = false;

        EEPROM_writeAnything(0, this->EepromBlock);
    }

    return 0;
}

void Settings::Assert(unsigned long avgScore) {
  for(uint8_t i = 0; i < 3; i++) {
    unsigned long old = this->EepromBlock.Scores[i];

    if (avgScore < old || old == 0) {
      this->EepromBlock.Scores[i] = avgScore;
      this->IsModified = true;

      if (i == 0) {
        unsigned long secondOld = this->EepromBlock.Scores[1];

        this->EepromBlock.Scores[1] = old;
        this->EepromBlock.Scores[2] = secondOld;
      }
      else if (i == 1)
        this->EepromBlock.Scores[2] = old;

      break;
    }
  }
}