

#include <Wire.h>           // Include the I2C library (required)
#include <SparkFunSX1509.h> //Click here for the library: http://librarymanager/All#SparkFun_SX1509
#include "LedControl.h"
#include <Bounce2.h>
#include <MIDIUSB.h>
#include <Smoothed.h> 




#include "LedControl.h"
#define LED_DIN 10
#define LED_CS 16
#define LED_CLK 14
#define NPOTS 4
#define CHN 15


LedControl lc=LedControl(LED_DIN,LED_CLK,LED_CS,1);
int delaytime=300;

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 io;                        // Create an SX1509 object to be used throughout


bool debugging=true;
bool hasChanged=false;

int potVals[NPOTS] = {0,0,0};
int lastPotVals[NPOTS] = {0,0,0};
int valz[NPOTS]={0,0,0};

boolean sliderPending=false;
boolean valueAnnounced=false;
int lastSlideVal=0;
int pendedAt=millis();

int drumBtns[4] = {15,2,1,0};

unsigned long time;




const byte maxPins=16;
const int slideButtonLevels[3] = {81,99,68};

int constCt=0;




Smoothed <int> Sensors[3];
int sensorCt = 3;
int aPins[3] = {A3,A1,A2};

Smoothed <int> sliderSensor;

boolean isIn(int array[], int element) {
 for (int i = 0; i < 4; i++) if (array[i] == element) return true;
 return false;
}

int getNearestPosition(int val)
{
  int distance = 128;
  int nearest = 0;
  int i;
  for(i=0;i<3;i++)
  {
    int tempdist = abs(val-slideButtonLevels[i]);
    if (tempdist<distance)
    {
      distance = tempdist;
      nearest = i;
    }
  }
  Serial.print("Nearest is ");Serial.print(nearest);Serial.print(" Val is ");Serial.println(val);
  return pow(2,(nearest+1));
}



byte lastVal[maxPins] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void setup()
{
  if (debugging) Serial.println("done");
  time = millis();
  
  // Serial is used in this example to display the input value
  // of the SX1509_INPUT_PIN input:
  if (debugging) Serial.begin(115200);

  Wire.begin();

  // Call io.begin(<address>) to initialize the SX1509. If it
  // successfully communicates, it'll return 1.
  if (io.begin(SX1509_ADDRESS) == false)
  {
    if (debugging) Serial.println("Failed to communicate. Check wiring and address of SX1509.");
    while (1)
      ; // If we fail to communicate, loop forever.
  }



  for (int i=0;i<maxPins;i++) io.pinMode(i, INPUT_PULLUP);

  for (int i=0;i<3;i++) Sensors[i].begin(SMOOTHED_AVERAGE,8);
  sliderSensor.begin(SMOOTHED_AVERAGE,10);

  setUpLED();
  hello();
  
  
}

void setUpLED()
{
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
}


void loop()
{

  
  //Serial.println("hello");
  readButtons();
  rPots();
  //readSlider();
  readSlide();

  

  
}



void readButtons()
{
  for (byte i=0;i<maxPins;i++)
  {
    int btnState = io.digitalRead(i);

    if (btnState!=lastVal[i]) 
    {
      lastVal[i]=btnState;
      hasChanged=true;
      //if (debugging) {Serial.print("Button "); Serial.println(i); Serial.print(":");Serial.println(btnState);}
      if (isIn(drumBtns,i))
      {
        if (debugging) {Serial.print("note on note off for "); Serial.println(i);}
        if (btnState==HIGH)  noteOn(0x00,0x24+i,0xD0);
        else noteOff(CHN,0x24+i,0xD0);
        MidiUSB.flush();
      }
      else
      {
        if (debugging) {Serial.print("control for"); Serial.println(i);}
        if (btnState==HIGH)  controlChange(0x00,0x30+i,0x1);
        else controlChange(CHN,0x30+i,0x60);
        MidiUSB.flush();
      }
    } 
    
  }
  if (hasChanged) {delay(50);hasChanged=false;}
}


bool diff(int a, int b)
{
  if (abs(a-b)>3) return true;
  return false;
}


void readSlider()
{
  
  sliderSensor.add(analogRead(A0)/60);
  int slideVal = quantize(sliderSensor.get());
  
  if (slideVal!=lastSlideVal )   makeChange(slideVal);
  
 
  
}

void makeChange(int sv)
{
  if (millis()-pendedAt <1000) return;
  if (debugging) Serial.print("The value is ");
  if (debugging) Serial.println(sv);
  pendedAt = millis();
  lastSlideVal=sv;
}

int quantize(int sv)
{
  
  for (int i=1;i<4;i++)
  {
    if (sv<=i*2)
    {
      return i;
    }
  }
  
}


void rPots()
{
       
    // Add the new value to both sensor value stores
    int val;
    
    for (int i=0;i<sensorCt;i++)
    {
      Sensors[i].add(analogRead(aPins[i]));
      val= Sensors[i].get()/8;
      //if (aPins[i]==A0) val= getNearestPosition(val);
      if (abs(val-lastPotVals[i])>2) 
        {
        hasChanged=true;
        lastPotVals[i]=val;
        printPotByPos(val,i);
        controlChange(CHN,60+i,val);
        }
      
    }
    if (hasChanged) {delay(5);hasChanged=false;}

}





void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  if (debugging) {Serial.print(control);Serial.print(":");Serial.println(value);}
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void printPotByPos(byte num,byte pos)
{
  
  byte numPerCent = min(100-(num*100/128),99);
  printDblDigitAt(numPerCent,pos);

}

void printDblDigitAt(byte nm, byte pos)
{
  char strToPrint[2] = {'0','0'};
  strToPrint[0] = '0'+ (nm%10);
  strToPrint[1] = '0'+ ((nm/10)%10);
  
  for (int i=0;i<2;i++) 
  {
    int at = 6-(2*pos)+i;
    lc.setChar(0,at,strToPrint[i],false);
  }
}




void hello(){
  int delaytime=300;
  lc.setChar(0,7,'H',false);
  lc.setChar(0,6,'E',false);
  lc.setChar(0,5,'L',false);
  lc.setChar(0,4,'L',false);
  lc.setChar(0,3,'0',false);
  lc.setChar(0,2,'.',false);
  lc.setChar(0,1,'.',false);
  lc.setChar(0,0,'.',false);
  delay(delaytime+1000);
  lc.clearDisplay(0);
  delay(delaytime);
  lc.setDigit(0,7,1,false);
  delay(delaytime);
  lc.setDigit(0,6,2,false);
  delay(delaytime);
  lc.setDigit(0,5,3,false);
  delay(delaytime);
  lc.setDigit(0,4,4,false);
  delay(delaytime);
  lc.setDigit(0,3,5,false);
  delay(delaytime);
  lc.setDigit(0,2,6,false);
  delay(delaytime);
  lc.setDigit(0,1,7,false);
  delay(delaytime);
  lc.setDigit(0,0,8,false);
  delay(1500);
  lc.clearDisplay(0);
  delay(delaytime);

  
  
}

void readSlide()
{
  
  
  int reading = quantize(round(analogRead(A0)/(float)50));
  if (reading!=lastSlideVal) {lastSlideVal=reading;   valueAnnounced=false;}    
  else
    {
      constCt++;
      if (constCt>10  && !valueAnnounced)
      {
        if (debugging) Serial.print("New Val is ");
        if (debugging) Serial.println(reading);
        constCt=0;
        valueAnnounced=true;
        int bars = round(pow(2,reading));
        if (debugging) Serial.println(bars);
        controlChange(CHN,70,bars);
        printDblDigitAt(bars,3);
      }
    }
  
}

void handleRotation()
{
  
//  bool vPotChanged=false;
//  bool tPotChanged=false;
//  tPotPos = tPot->getVal();
//  vPotPos = vPot->getVal();
//
//  //Serial.print(tPotPos);
//
//  if (oldTPotPos!=tPotPos) {
//    oldTPotPos = tPotPos;
//    tPotChanged=true;
//  }
//
//  if (oldVPotPos!=vPotPos) {
//    oldVPotPos = vPotPos;   
//    vPotChanged=true;
//  }
//
//  if (tPotChanged || vPotChanged) 
//  {
//    Serial.print("T Pot Val: ");
//    Serial.println(tPotPos);
//    Serial.print("V Pot Val: ");
//    Serial.println(vPotPos);
//  }
}

void readPots()
{
  bool hasChanged=false;
  for (int i=0;i<3;i++)
  {
    int val = analogRead(aPins[i])/8;
    if (val!=lastPotVals[i]) 
      {
        //controlChange(0x00,0x40+i,0x7F-val);
        hasChanged=true;
        lastPotVals[i]=val;
        //printNumToLed(val,i-A2);
        printPotByPos(val,i);
        }
    
  }
  if (hasChanged)  
  {
    delay(5);
    //MidiUSB.flush();
  }
}
