#ifndef MBUSPARSER_H
#define MBUSPARSER_H

#include "stdbool.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

struct MeterData {
  // Active Power +/-
  uint32_t activePowerPlus;
  bool activePowerPlusValid;
  uint32_t activePowerMinus;
  bool activePowerMinusValid;
  uint32_t activePowerPlusL1;
  bool activePowerPlusValidL1;
  uint32_t activePowerMinusL1;
  bool activePowerMinusValidL1;
  uint32_t activePowerPlusL2;
  bool activePowerPlusValidL2;
  uint32_t activePowerMinusL2;
  bool activePowerMinusValidL2;
  uint32_t activePowerPlusL3;
  bool activePowerPlusValidL3;
  uint32_t activePowerMinusL3;
  bool activePowerMinusValidL3;

  // Reactive Power +/-
  uint32_t reactivePowerPlus;
  bool reactivePowerPlusValid;
  uint32_t reactivePowerMinus;
  bool reactivePowerMinusValid;

  // Voltage L1/L2/L3
  uint32_t voltageL1;
  bool voltageL1Valid;
  uint32_t voltageL2;
  bool voltageL2Valid;
  uint32_t voltageL3;
  bool voltageL3Valid;

  // Current L1/L2/L3
  uint32_t centiAmpereL1;
  bool centiAmpereL1Valid;
  uint32_t centiAmpereL2;
  bool centiAmpereL2Valid;
  uint32_t centiAmpereL3;
  bool centiAmpereL3Valid;

  //Energy
  uint32_t activeImportWh;
  bool activeImportWhValid;
  uint32_t activeExportWh;
  bool activeExportWhValid;
  uint32_t activeImportWhL1;
  bool activeImportWhValidL1;
  uint32_t activeExportWhL1;
  bool activeExportWhValidL1;
  uint32_t activeImportWhL2;
  bool activeImportWhValidL2;
  uint32_t activeExportWhL2;
  bool activeExportWhValidL2;
  uint32_t activeImportWhL3;
  bool activeImportWhValidL3;
  uint32_t activeExportWhL3;
  bool activeExportWhValidL3;

  uint32_t reactiveImportWh;
  bool reactiveImportWhValid;
  uint32_t reactiveExportWh;
  bool reactiveExportWhValid;

  //Powerfactor
  uint32_t powerFactorL1;
  bool powerFactorValidL1;
  uint32_t powerFactorL2;
  bool powerFactorValidL2;
  uint32_t powerFactorL3;
  bool powerFactorValidL3;
  uint32_t powerFactorTotal;
  bool powerFactorTotalValid;
  
  uint8_t listId;
  size_t parseResultBufferSize;
  size_t parseResultMessageSize;
  struct tm DateTime;
};

struct MeterData parseMbusFrame(unsigned char *frame, int fLen);

#endif
