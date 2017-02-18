#ifndef TEMP_MGR_H
#define TEMP_MGR_H

#include "application.h"

#define NO_DATA -999

class TempMgr {
public:
  TempMgr(int count);

  void addTempSample(int index, float temp);
  int getNumSamples(int index);
  float getAverageTemp(int index);

private:
  float* totals;
  int* numSamples;
};

#endif
