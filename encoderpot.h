#ifndef encoderpot_h
#define encoderpot_h

#include "Arduino.h"
#include <Bounce2.h>

class encoderpot
{
  public:
    encoderpot(int  pinA, int  pinB, int min, int max, int increment);
    int getVal();
    void setVal(int valToSet);
  private:
    int _val,_valA,_valB, _increment,_min, _max;
    int  _pinA,_pinB;
    bool _motionDetected, _isCW;
    Bounce _dbA= Bounce(); 
    Bounce _dbB= Bounce(); 
};
#endif
