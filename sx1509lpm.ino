

#include <Wire.h>           // Include the I2C library (required)
#include <SparkFunSX1509.h> //Click here for the library: http://librarymanager/All#SparkFun_SX1509
#include "LedControl.h"
#include "encoderpot.h"
#include <Bounce2.h>
#include <MIDIUSB.h>
#include <Smoothed.h> 


#include "LedControl.h"
#define LED_DIN 10
#define LED_CS 16
#define LED_CLK 14
#define NPOTS 4
LedControl lc=LedControl(LED_DIN,LED_CLK,LED_CS,1);
int delaytime=300;

// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 io;                        // Create an SX1509 object to be used throughout


bool debugging=true;
bool hasChanged=false;

int potVals[NPOTS] = {0,0,0,0};
int lastPotVals[NPOTS] = {0,0,0,0};

int lastSlideVal=0;

const byte maxPins=16;
const int slideButtonLevels[4] = {15,18,22,31};




//Smoothed <int> a2Sensor; 
//Smoothed <int> a3Sensor;
//Smoothed <int> a0Sensor;
//Smoothed <int> a1Sensor;


Smoothed <int> Sensors[4];





byte lastVal[maxPins] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void setup()
{
  
  // Serial is used in this example to display the input value
  // of the SX1509_INPUT_PIN input:
  Serial.begin(115200);

  Wire.begin();

  // Call io.begin(<address>) to initialize the SX1509. If it
  // successfully communicates, it'll return 1.
  if (io.begin(SX1509_ADDRESS) == false)
  {
    Serial.println("Failed to communicate. Check wiring and address of SX1509.");
    while (1)
      ; // If we fail to communicate, loop forever.
  }



  for (int i=0;i<maxPins;i++) io.pinMode(i, INPUT_PULLUP);

  for (int i=0;i<NPOTS;i++) Sensors[i].begin(SMOOTHED_AVERAGE,10);

  setUpLED();
  hello();
  
  
}

void setUpLED()
{
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
}

void readSlideButton()
{ 
  int newSlideVal = (int)sqrt(analogRead(A0));
  if (newSlideVal!=lastSlideVal)
  {
    lastSlideVal = newSlideVal;
    Serial.println(getBars(newSlideVal));
  }
  
}

int getBars(int btnVal)
{
  for (int i=0;i<4;i++) if (btnVal==slideButtonLevels[i])  return round(pow(2,(i+1)));
}

void loop()
{

  //handleRotation();
  //rPots();
  rPots();
  readButtons();
  //readSlideButton();

  

  
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
      if (debugging) {Serial.print("Button "); Serial.println(i); Serial.print(":");Serial.println(btnState);}
    } 
    
  }
  if (hasChanged) {delay(50);hasChanged=false;}
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

bool diff(int a, int b)
{
  if (abs(a-b)>3) return true;
  return false;
}

void readPots()
{
  bool hasChanged=false;
  for (int i=A0;i<=A3;i++)
  {
    int val = analogRead(i)/8;
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

void rPots()
{
       
    // Add the new value to both sensor value stores
    
    int valz[4] = {0,0,0,0};
    for (int i=0;i<4;i++)
    {
      Sensors[i].add(analogRead(A0+i));
      valz[i]= Sensors[i].get()/8;
      if (diff(valz[i],lastPotVals[i])) 
        {
        hasChanged=true;
        lastPotVals[i]=valz[i];
        printPotByPos(valz[i],i);
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
  char strToPrint[2] = {'0','0'};
  byte numPerCent = min(100-(num*100/128),99);
  strToPrint[1] = '0'+ (numPerCent%10);
  strToPrint[0] = '0'+ ((numPerCent/10)%10);
  
  for (int i=0;i<2;i++) 
  {
    int at = ((pos+1)*2)-(i+1);
    lc.setChar(0,at,strToPrint[i],false);
  }

}


void aprintNumToLed(byte num, bool left)
{
   char strToPrint[3] = {'0','0','0'};

     
   strToPrint[2] = '0'+ (num%10);
  
   if (num>9) strToPrint[1] = '0'+ ((num/10)%10);   
   if (num>99) strToPrint[0] = '0'+ ((num/100)%10);

   for (int i=0;i<3;i++)
   {
      if (!left) lc.setChar(0,2-i,strToPrint[i],false);
      else lc.setChar(0,6-i,strToPrint[i],false);
   }
  

  //if (debugging) Serial.print("num is: ");
  //if (debugging) Serial.println(num);
  //if (debugging) Serial.print("Position is: ");
  //if (debugging) Serial.println(left);
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
