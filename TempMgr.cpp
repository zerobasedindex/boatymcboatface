#include "TempMgr.h"

TempMgr::TempMgr(int count) {
  totals = new float[count];
  numSamples = new int[count];

  for (int i = 0; i < count; i++) {
    totals[i] = 0;
    numSamples[i] = 0;
  }
}

void TempMgr::addTempSample(int index, float temp) {
  totals[index] += temp;
  numSamples[index]++;
}

int TempMgr::getNumSamples(int index) {
  return numSamples[index];
}

float TempMgr::getAverageTemp(int index) {
  if (numSamples[index] > 0) {
    float avg = totals[index] / (float)numSamples[index];
    totals[index] = 0.0;
    numSamples[index] = 0;
    return avg;
  } else {
    return NO_DATA;
  }
}
