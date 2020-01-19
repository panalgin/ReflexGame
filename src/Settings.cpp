#include "Settings.h"
#include "EEPROMAnything.h"

/// Base constructor
Settings::Settings() {}

/*
 * Reads and loads setting values from the eeprom
 */
uint8_t Settings::Load(void) {
    EEPROM_readAnything(0, EepromBlock);

    if (strcmp(this->EepromBlock.Version, "129") != 0) {
        strcpy(this->EepromBlock.Version, "129");
        this->EepromBlock.Scores[0] = UINT16_MAX;
        this->EepromBlock.Scores[1] = UINT16_MAX;
        this->EepromBlock.Scores[2] = UINT16_MAX;
        this->EepromBlock.Scores[3] = UINT16_MAX;
        this->EepromBlock.Scores[4] = UINT16_MAX;

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

int compare(const void* a, const void* b) {
    int result = 0;

    if (*(uint16_t*)a < *(uint16_t*)b) 
        result = -1;
    else if (*(uint16_t*)a == *(uint16_t*)b) 
        result = 0;
    else if (*(uint16_t*)a > *(uint16_t*)b) 
        result = 1;

    return result;
}

void Settings::Assert(uint16_t avgScore) {
    uint16_t scores[] = {
        this->EepromBlock.Scores[0], this->EepromBlock.Scores[1],
        this->EepromBlock.Scores[2], this->EepromBlock.Scores[3],
        this->EepromBlock.Scores[4], avgScore};

    qsort(scores, 6, sizeof(uint16_t), compare);

    for (uint8_t n = 0; n < 6; n++) {
        Serial.print(scores[n]);
        Serial.print(", ");
    }

    Serial.println("");

    for (uint8_t i = 0; i < 5; i++) {
        uint16_t old = this->EepromBlock.Scores[i];

        if (scores[i] != old) {
            this->EepromBlock.Scores[i] = scores[i];
            this->IsModified = true;
        }
    }
}