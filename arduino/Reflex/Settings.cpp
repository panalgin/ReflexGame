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
  case 0:
    sprintf(result, "%u secs", this->EepromBlock.DominationGameDuration);

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

  default:
    break;
  }
}
