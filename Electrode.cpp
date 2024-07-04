#include "Electrode.h"

Electrode::Electrode(int n) {
    number = n;
    domFreq = 0;
}

Electrode::~Electrode() {

}

//function measures the dominant frequency of an electrode site
//each call to the function serves as measuring for 1 minute, and determining
//  dominant frequency of any given electrode site
int Electrode::measure(int max, int min) {
  float frequency;
  //initialization values for determining cominant frequency
  max = max * 2 * PI; //max period/frequency
  min = min * 2 * PI; //min period/frequency
  int dominantFrequency = 0;
  map[0] = 0;
  //---------------------------------------------------------
  for (int i = 0; i < 120; i++) {
      int period = generateWave(i, max, min);
      frequency = period / (2 * PI);

      if (map.count(frequency) == 0) {
        map[frequency] = 1;
      } else {
        map[frequency] = map.at(frequency) + 1;
      }
      if (map.at(frequency) > map.at(dominantFrequency)) {
        dominantFrequency = frequency;
      }
  }
  domFreq = dominantFrequency;
  return dominantFrequency;
}

//**this is an internal/helper function only, please do not directly call**
//function generates a random brainwave with realistic properties.
//Returns an integer value for the voltage value at a set time values

//Currently only returns the period value, as that is all that is needed for the
//   assignmnt, but the option to return the wave equation is implimented, you
//   just have to change the return value to wave.

//Takes and int i (a time value), and a max and min (which correspond to a specific
//  waveform inputed for testing)
int Electrode::generateWave(int i, int max, int min) {
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<int> periodDist(min, max); //period
  uniform_int_distribution<int> amplitudeDist(1, 200); //voltage

  int period = periodDist(gen);
  int amplitude = amplitudeDist(gen);

  int wave = amplitude * sin(period * i);
  wave = abs(wave);
  return period;
}

int Electrode::administerTreatment(int frequency) {
  return domFreq + frequency;
}
