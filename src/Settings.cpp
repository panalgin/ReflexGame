#include "Settings.h"
#include "EEPROMAnything.h"

/// Base constructor
Settings::Settings() {}

/*
 * Reads and loads setting values from the eeprom
 */
uint8_t Settings::Load(void)
{
  EEPROM_readAnything(0, EepromBlock);

  if (strcmp(this->EepromBlock.Version, "123") != 0)
  {
    strcpy(this->EepromBlock.Version, "123");
    this->EepromBlock.BombGameDuration = 20;
    strcpy(this->EepromBlock.BombPassword, "1234567");
    this->EepromBlock.BombRfidDefuseDuration = 5;
    this->EepromBlock.BombExplodesOnWrongEntry = 0;

    this->EepromBlock.DominationGameDuration = 20;
    this->EepromBlock.DominationRfidDuration = 5;
    this->EepromBlock.DominationGameMode = 0;

    this->EepromBlock.DominationStartUpDelay = 0;
    this->EepromBlock.DominationPressureDuration = 1;
    this->EepromBlock.NeutralizationPressureDuration = 1;

    EEPROM_writeAnything(0, this->EepromBlock);
  }

#ifdef _DEBUG
  //Print eeprom values
#endif

  return 0;
}

uint8_t Settings::Save(void)
{
  if (this->IsModified)
  {
    this->IsModified = false;

    EEPROM_writeAnything(0, this->EepromBlock);
  }

  return 0;
}

bool Settings::SetValue(uint8_t index, char *value)
{
  uint16_t iValue = index == 3 ? strlen(value) : (uint16_t)atol(value); // If the property is bombpassword, we only check the length

  if (iValue < this->PropertyLimits[index][0])
  {
    this->LastErrorCode = 2; // Value was smaller than minimum allowed
    return false;
  }
  else if (iValue > this->PropertyLimits[index][1])
  {
    this->LastErrorCode = 1; // Value was bigger than maximum allowed
    return false;
  }
  else
  {
    this->SetValueRaw(index, value);
    this->LastErrorCode = 0;
    return true;
  }
}

/// Gets screen-friendly value of the given parameter by order in EepromBlock
char *Settings::GetValue(uint8_t index)
{
  char *result = (char *)malloc(sizeof(char) * 14);

  switch (index)
  {
  case 0: // Domination Game Duration // Overall
    sprintf(result, "%u secs", this->EepromBlock.DominationGameDuration);

    break;

  case 1: // Domination Game Mode // 0= Fixed Domination Period, 1= Fixed Game Duration
    sprintf(result, "%d", this->EepromBlock.DominationGameMode);

    break;

  case 2: // Domination Rfid Duration
    sprintf(result, "%d secs", this->EepromBlock.DominationRfidDuration);

    break;

  case 3: // Bomb Game Password
    sprintf(result, "%s", this->EepromBlock.BombPassword);
    break;

  case 4: // Bomb Game Duration - Countdown
    sprintf(result, "%d secs", this->EepromBlock.BombGameDuration);
    break;

  case 5: // Bomb Explodes On Wrong Entry
    sprintf(result, "%d", this->EepromBlock.BombExplodesOnWrongEntry);

    break;

  case 6: // Bomb Rfid Defuse Duration
    sprintf(result, "%d secs", this->EepromBlock.BombRfidDefuseDuration);

    break;

  case 7: // Domination Start Up Delay
    sprintf(result, "%d secs", this->EepromBlock.DominationStartUpDelay);

    break;

  case 8: // Domination Pressure Duration
    sprintf(result, "%d secs", this->EepromBlock.DominationPressureDuration);

    break;

  case 9: // Neutralization Pressure Duration
    sprintf(result, "%d secs", this->EepromBlock.NeutralizationPressureDuration);

    break;

  default:
    strcpy(result, "Error");

    break;
  }

  return result;
}

void Settings::SetValueRaw(uint8_t index, char *value)
{
  this->IsModified = true;

  uint16_t iValue = (uint16_t)atol(value);

  switch (index)
  {
  case 0:
    this->EepromBlock.DominationGameDuration = iValue;
    break;
  case 1:
    this->EepromBlock.DominationGameMode = (uint8_t)iValue;
    break;
  case 2:
    this->EepromBlock.DominationRfidDuration = (uint8_t)iValue;
    break;
  case 3:
    strcpy(this->EepromBlock.BombPassword, value);
    break;
  case 4:
    this->EepromBlock.BombGameDuration = iValue;
    break;
  case 5:
    this->EepromBlock.BombExplodesOnWrongEntry = (uint8_t)iValue;
    break;
  case 6:
    this->EepromBlock.BombRfidDefuseDuration = (uint8_t)iValue;
    break;

  case 7:
    this->EepromBlock.DominationStartUpDelay = (uint8_t)iValue;
    break;

  case 8:
    this->EepromBlock.DominationPressureDuration = iValue;
    break;

  case 9:
    this->EepromBlock.NeutralizationPressureDuration = iValue;
    break;

  default:
    break;
  }
}
