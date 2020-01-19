#pragma once
#ifndef EEPROMBlock_h
#define EEPROMBlock_h

#include <Arduino.h>

struct config_t {
    char Version[4];
    unsigned long Scores[5];
};
#endif