#include <string.h>
#include "mbusparser.h"
#include "vec.h"

void initMeterData(struct MeterData *MD) {
  MD->activePowerPlus = 0;
  MD->activePowerPlusValid = false;
  MD->activePowerMinus = 0;
  MD->activePowerMinusValid = false;
  MD->activePowerPlusL1 = 0;
  MD->activePowerPlusValidL1 = false;
  MD->activePowerMinusL1 = 0;
  MD->activePowerMinusValidL1 = false;
  MD->activePowerPlusL2 = 0;
  MD->activePowerPlusValidL2 = false;
  MD->activePowerMinusL2 = 0;
  MD->activePowerMinusValidL2 = false;
  MD->activePowerPlusL3 = 0;
  MD->activePowerPlusValidL3 = false;
  MD->activePowerMinusL3 = 0;
  MD->activePowerMinusValidL3 = false;

  // Reactive Power +/-
  MD->reactivePowerPlus = 0;
  MD->reactivePowerPlusValid = false;
  MD->reactivePowerMinus = 0;
  MD->reactivePowerMinusValid = false;

  // Voltage L1/L2/L3
  MD->voltageL1 = 0;
  MD->voltageL1Valid = false;
  MD->voltageL2 = 0;
  MD->voltageL2Valid = false;
  MD->voltageL3 = 0;
  MD->voltageL3Valid = false;

  // Current L1/L2/L3
  MD->centiAmpereL1 = 0;
  MD->centiAmpereL1Valid = false;
  MD->centiAmpereL2 = 0;
  MD->centiAmpereL2Valid = false;
  MD->centiAmpereL3 = 0;
  MD->centiAmpereL3Valid = false;

  //Energy
  MD->activeImportWh = 0;
  MD->activeImportWhValid = false;
  MD->activeExportWh = 0;
  MD->activeExportWhValid = false;
  MD->activeImportWhL1 = 0;
  MD->activeImportWhValidL1 = false;
  MD->activeExportWhL1 = 0;
  MD->activeExportWhValidL1 = false;
  MD->activeImportWhL2 = 0;
  MD->activeImportWhValidL2 = false;
  MD->activeExportWhL2 = 0;
  MD->activeExportWhValidL2 = false;
  MD->activeImportWhL3 = 0;
  MD->activeImportWhValidL3 = false;
  MD->activeExportWhL3 = 0;
  MD->activeExportWhValidL3 = false;

  MD->reactiveImportWh = 0;
  MD->reactiveImportWhValid = false;
  MD->reactiveExportWh = 0;
  MD->reactiveExportWhValid = false;

  //Powerfactor
  MD->powerFactorL1 = 0;
  MD->powerFactorValidL1 = false;
  MD->powerFactorL2 = 0;
  MD->powerFactorValidL2 = false;
  MD->powerFactorL3 = 0;
  MD->powerFactorValidL3 = false;
  MD->powerFactorTotal = 0;
  MD->powerFactorTotalValid = false;
  
  MD->listId = 0;
  MD->parseResultBufferSize = 0;
  MD->parseResultMessageSize = 0;
}


size_t find(unsigned char *f, unsigned char *needle, uint8_t fLen, uint8_t nLen)
{
  for (uint8_t* it = f; it < (f + fLen - sizeof(needle)); ++it) {
    if (memcmp(it, needle, nLen) == 0) {
      return it - f;
    }
  }
  return -1;
}

uint32_t getObisValue(unsigned char *frame,
                      int fLen,
                      uint8_t codeA,
                      uint8_t codeB,
                      uint8_t codeC,
                      uint8_t codeD,
                      uint8_t codeE,
                      uint8_t codeF,
                      uint8_t size,
                      bool *success)
{
  *success = false;
  uint8_t theObis[8] = { 0x09, 0x06 };
  theObis[2] = codeA;
  theObis[3] = codeB;
  theObis[4] = codeC;
  theObis[5] = codeD;
  theObis[6] = codeE;
  theObis[7] = codeF;

  int indexOfData = find(frame, theObis, fLen, sizeof(theObis));

  if (indexOfData >= 0) {
    const uint8_t *theBytes = frame + indexOfData + sizeof(theObis) + 1;

    if (size == 2) {
      *success = true;
      return
        (uint32_t)theBytes[0] << (8*1) |
        (uint32_t)theBytes[1];
    } else if (size == 3) {
      *success = true;
      return
        (uint32_t)theBytes[0] << (8*2) |
        (uint32_t)theBytes[1] << (8*1) |
        (uint32_t)theBytes[2];
    } else if (size == 4) {
      *success = true;
      return
        (uint32_t)theBytes[0] << (8*3) |
        (uint32_t)theBytes[1] << (8*2) |
        (uint32_t)theBytes[2] << (8*1) |
        (uint32_t)theBytes[3];
    }
  }
  return 0;
}

enum PowerType {
  ACTIVE_IMPORT,
  ACTIVE_EXPORT,
  REACTIVE_IMPORT,
  REACTIVE_EXPORT,
  ACTIVE_POWER_PLUS,
  ACTIVE_POWER_MINUS,
  REACTIVE_POWER_PLUS,
  REACTIVE_POWER_MINUS,
  VOLTAGE_L1,
  VOLTAGE_L2,
  VOLTAGE_L3,
  CURRENT_L1,
  CURRENT_L2,
  CURRENT_L3,
  ACTIVE_POWER_PLUS_L1,
  ACTIVE_POWER_PLUS_L2,
  ACTIVE_POWER_PLUS_L3,
  POWER_FACTOR_L1,
  POWER_FACTOR_L2,
  POWER_FACTOR_L3,
  POWER_FACTOR,
  ACTIVE_POWER_MINUS_L1,
  ACTIVE_POWER_MINUS_L2,
  ACTIVE_POWER_MINUS_L3,
  ACTIVE_EXPORT_L1,
  ACTIVE_EXPORT_L2,
  ACTIVE_EXPORT_L3,
  ACTIVE_IMPORT_L1,
  ACTIVE_IMPORT_L2,
  ACTIVE_IMPORT_L3
};

uint32_t getPower(unsigned char *frame, int fLen, enum PowerType type, bool *success)
{
  switch (type) {
    case ACTIVE_IMPORT:        return getObisValue(frame, fLen, 1, 1, 1, 8, 0, 0xff, 4, success);
    case ACTIVE_EXPORT:        return getObisValue(frame, fLen, 1, 1, 2, 8, 0, 0xff, 4, success);
    case REACTIVE_IMPORT:      return getObisValue(frame, fLen, 1, 1, 3, 8, 0, 0xff, 4, success);
    case REACTIVE_EXPORT:      return getObisValue(frame, fLen, 1, 1, 4, 8, 0, 0xff, 4, success);
    case ACTIVE_POWER_PLUS:    return getObisValue(frame, fLen, 1, 1, 1, 7, 0, 0xff, 4, success);
    case ACTIVE_POWER_MINUS:   return getObisValue(frame, fLen, 1, 1, 2, 7, 0, 0xff, 4, success);
    case REACTIVE_POWER_PLUS:  return getObisValue(frame, fLen, 1, 1, 3, 7, 0, 0xff, 4, success);
    case REACTIVE_POWER_MINUS: return getObisValue(frame, fLen, 1, 1, 4, 7, 0, 0xff, 4, success);
    case VOLTAGE_L1:           return getObisValue(frame, fLen, 1, 1, 32, 7, 0, 0xff, 2, success);
    case VOLTAGE_L2:           return getObisValue(frame, fLen, 1, 1, 52, 7, 0, 0xff, 2, success);
    case VOLTAGE_L3:           return getObisValue(frame, fLen, 1, 1, 72, 7, 0, 0xff, 2, success);
    case CURRENT_L1:           return getObisValue(frame, fLen, 1, 1, 31, 7, 0, 0xff, 4, success);
    case CURRENT_L2:           return getObisValue(frame, fLen, 1, 1, 51, 7, 0, 0xff, 4, success);
    case CURRENT_L3:           return getObisValue(frame, fLen, 1, 1, 71, 7, 0, 0xff, 4, success);
    case ACTIVE_POWER_PLUS_L1: return getObisValue(frame, fLen, 1, 1, 21, 7, 0, 0xff, 4, success);
    case ACTIVE_POWER_PLUS_L2: return getObisValue(frame, fLen, 1, 1, 41, 7, 0, 0xff, 4, success);
    case ACTIVE_POWER_PLUS_L3: return getObisValue(frame, fLen, 1, 1, 61, 7, 0, 0xff, 4, success);
    case POWER_FACTOR_L1:      return getObisValue(frame, fLen, 1, 1, 33, 7, 0, 0xff, 2, success);
    case POWER_FACTOR_L2:      return getObisValue(frame, fLen, 1, 1, 53, 7, 0, 0xff, 2, success);
    case POWER_FACTOR_L3:      return getObisValue(frame, fLen, 1, 1, 73, 7, 0, 0xff, 2, success);
    case POWER_FACTOR:         return getObisValue(frame, fLen, 1, 1, 13, 7, 0, 0xff, 2, success); 
    case ACTIVE_POWER_MINUS_L1:return getObisValue(frame, fLen, 1, 1, 22, 7, 0, 0xff, 4, success);
    case ACTIVE_POWER_MINUS_L2:return getObisValue(frame, fLen, 1, 1, 42, 7, 0, 0xff, 4, success);
    case ACTIVE_POWER_MINUS_L3:return getObisValue(frame, fLen, 1, 1, 62, 7, 0, 0xff, 4, success); 
    case ACTIVE_EXPORT_L1:     return getObisValue(frame, fLen, 1, 1, 22, 8, 0, 0xff, 4, success);
    case ACTIVE_EXPORT_L2:     return getObisValue(frame, fLen, 1, 1, 42, 8, 0, 0xff, 4, success);
    case ACTIVE_EXPORT_L3:     return getObisValue(frame, fLen, 1, 1, 62, 8, 0, 0xff, 4, success);
    case ACTIVE_IMPORT_L1:     return getObisValue(frame, fLen, 1, 1, 21, 8, 0, 0xff, 4, success);
    case ACTIVE_IMPORT_L2:     return getObisValue(frame, fLen, 1, 1, 41, 8, 0, 0xff, 4, success);
    case ACTIVE_IMPORT_L3:     return getObisValue(frame, fLen, 1, 1, 61, 8, 0, 0xff, 4, success);
  default: return 0;
  }
}

struct MeterData result;
struct MeterData parseMbusFrame(unsigned char *frame, int fLen)
{
  initMeterData(&result);
  unsigned char frameFormat = ((frame[1]) & 0xF0);
  size_t messageSize = (((frame[1] & 0x0F) << 8) | frame[2]);
  result.parseResultBufferSize = fLen;
  result.parseResultMessageSize = messageSize;
  unsigned char needle[] = { 0xff, 0x80, 0x00, 0x00 };
  size_t dateTimeEnd = find(frame, needle, fLen, sizeof(needle));
  if (dateTimeEnd > 0) {
    result.listId = (frame[dateTimeEnd + 5] & 0xF0) >> 4;
  }

  if (frame[0] == 0x7E && (frame[fLen - 1]) == 0x7E) {
    if (frameFormat == 0xA0) {
      // TODO: Parse header
      // TODO: Parse datetime
      // TODO: Parse elements sequentially
      result.activePowerPlus = getPower(frame, fLen, ACTIVE_POWER_PLUS, &result.activePowerPlusValid);
      result.activePowerMinus = getPower(frame, fLen, ACTIVE_POWER_MINUS, &result.activePowerMinusValid);
      result.reactivePowerPlus = getPower(frame, fLen, REACTIVE_POWER_PLUS, &result.reactivePowerPlusValid);
      result.reactivePowerMinus = getPower(frame, fLen, REACTIVE_POWER_MINUS, &result.reactivePowerMinusValid);
      result.centiAmpereL1 = getPower(frame, fLen, CURRENT_L1, &result.centiAmpereL1Valid);
      result.centiAmpereL2 = getPower(frame, fLen, CURRENT_L2, &result.centiAmpereL2Valid);
      result.centiAmpereL3 = getPower(frame, fLen, CURRENT_L3, &result.centiAmpereL3Valid);
      result.voltageL1 = getPower(frame, fLen, VOLTAGE_L1, &result.voltageL1Valid);
      result.voltageL2 = getPower(frame, fLen, VOLTAGE_L2, &result.voltageL2Valid);
      result.voltageL3 = getPower(frame, fLen, VOLTAGE_L3, &result.voltageL3Valid);
      result.activeImportWh = getPower(frame, fLen, ACTIVE_IMPORT, &result.activeImportWhValid)*10;
      result.activeExportWh = getPower(frame, fLen, ACTIVE_EXPORT, &result.activeExportWhValid)*10;
      result.reactiveImportWh = getPower(frame, fLen, REACTIVE_IMPORT, &result.reactiveImportWhValid)*10;
      result.reactiveExportWh = getPower(frame, fLen, REACTIVE_EXPORT, &result.reactiveExportWhValid)*10;
      result.activePowerPlusL1 = getPower(frame, fLen, ACTIVE_POWER_PLUS_L1, &result.activePowerPlusValidL1);
      result.activePowerPlusL2 = getPower(frame, fLen, ACTIVE_POWER_PLUS_L2, &result.activePowerPlusValidL2);
      result.activePowerPlusL3 = getPower(frame, fLen, ACTIVE_POWER_PLUS_L3, &result.activePowerPlusValidL3);
      result.activePowerMinusL1 = getPower(frame, fLen, ACTIVE_POWER_MINUS_L1, &result.activePowerMinusValidL1);
      result.activePowerMinusL2 = getPower(frame, fLen, ACTIVE_POWER_MINUS_L2, &result.activePowerMinusValidL2);
      result.activePowerMinusL3 = getPower(frame, fLen, ACTIVE_POWER_MINUS_L3, &result.activePowerMinusValidL3);
      result.powerFactorL1 = getPower(frame, fLen, POWER_FACTOR_L1, &result.powerFactorValidL1);
      result.powerFactorL2 = getPower(frame, fLen, POWER_FACTOR_L2, &result.powerFactorValidL2);
      result.powerFactorL3 = getPower(frame, fLen, POWER_FACTOR_L3, &result.powerFactorValidL3);
      result.powerFactorTotal = getPower(frame, fLen, POWER_FACTOR, &result.powerFactorTotalValid);
      result.activeImportWhL1 = getPower(frame, fLen, ACTIVE_IMPORT_L1, &result.activeImportWhValidL1);
      result.activeImportWhL2 = getPower(frame, fLen, ACTIVE_IMPORT_L2, &result.activeImportWhValidL2);
      result.activeImportWhL3 = getPower(frame, fLen, ACTIVE_IMPORT_L3, &result.activeImportWhValidL3);
      result.activeExportWhL1 = getPower(frame, fLen, ACTIVE_EXPORT_L1, &result.activeExportWhValidL1);
      result.activeExportWhL2 = getPower(frame, fLen, ACTIVE_EXPORT_L2, &result.activeExportWhValidL2);
      result.activeExportWhL3 = getPower(frame, fLen, ACTIVE_EXPORT_L3, &result.activeExportWhValidL3);
    }
  }
  return result;
}
