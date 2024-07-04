#ifndef ELECTRODE_H
#define ELECTRODE_H

#include <iostream>
#include <unordered_map>
#include <cmath>
#include <random>
#include <time.h>
#include <unistd.h>
#include "defs.h"
#include <string>

using namespace std;

class Electrode {
  public:
    Electrode(int);
    ~Electrode();
    int measure(int, int);
    int generateWave(int, int, int);
    int administerTreatment(int);

  private:
    int number;
    int domFreq;
    unordered_map<int, int> map;
};

#endif
