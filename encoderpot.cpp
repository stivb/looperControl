/*

*/

#include "Arduino.h"
#include <Bounce2.h>
#include "encoderpot.h"

encoderpot::encoderpot(int  pinA, int  pinB, int min, int max, int increment)
{
  
  _pinA=pinA;
  _pinB=pinB;
  pinMode(_pinA,INPUT_PULLUP);
  pinMode(_pinB,INPUT_PULLUP);
  _dbA.attach(_pinA);
  _dbA.interval(5);
  _dbB.attach(_pinB);
  _dbB.interval(5);
  _min=min;
  _max=max;
  _increment=increment;
  
}

void encoderpot::setVal(int valToSet)
{
  _val=valToSet;
}

int encoderpot::getVal()
{

  _dbA.update();
  _dbB.update();
  _valA = _dbA.read();
  _valB = _dbB.read();
  //Serial.print(_valA);Serial.println(_valB);
  
  if (_valA && _valB && _motionDetected ) //in a detent and just arrived
  {
    if (_isCW) _val = _val + _increment;
    else _val= _val - _increment;
    if (_val>_max) _val=_min;
    if (_val<_min) _val=_max;
    _motionDetected = false;
    Serial.println("detent");
  }

  if (_valA  && !_valB  && !_motionDetected ) // just started CW
  {
    _isCW= true;
    _motionDetected=true;
    Serial.println("cw");
  }

  if (!_valA && _valB && !_motionDetected )  //just started CCW
  {
    _isCW= false;
    _motionDetected=true;
    Serial.println("ccw");
  }

  return _val;
} 
