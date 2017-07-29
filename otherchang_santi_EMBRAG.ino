#include <Servo.h>

#define GEARCHANGE 9
#define CLUTCH 8
#define GEARPOS A0
#define CLUTCHPOS A1
#define CLUTCHPOT A2
#define CLUTCHBUT 4
#define NEUTBUT A3
#define NEUTRAL 5

#define POT_MAX 1023
#define POT_MIN 0

#define UPCHANGE 2
#define DOWNCHANGE 3

#define DELAY 600

#define midpoint 90
#define midpointms 1450
//#define halfup 65
#define halfup 1330
//#define downchange 180
#define downchange 2100
//#define upchange 0
#define upchange 900
#define movetime 500

//#define clutched 0
//#define declutched 100

#define clutched 900
#define declutched 2100

Servo gearchg;
Servo clutch;

int gear = 0;

unsigned long lastmove = 0;
int currmove = 0;

int change = 0;
int changeinprogress = 0;
unsigned long lastshow = 0;
int nowclutched = 0;
unsigned long lastclutched = 0;

unsigned long laststart = 0;
int started = 0;

int RELEASE = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(GEARCHANGE, OUTPUT);
  pinMode(CLUTCH, OUTPUT);
  pinMode(UPCHANGE, INPUT);
  pinMode(DOWNCHANGE, INPUT);
  pinMode(NEUTBUT, INPUT);
  pinMode(CLUTCHBUT, INPUT);

  gearchg.attach(GEARCHANGE);
  clutch.attach(CLUTCH);
  gearchg.writeMicroseconds(midpointms);
  clutch.write(clutched);
}

void loop()
{

  if (digitalRead(CLUTCHBUT) == 0)
  {
    clutch.write(declutched);
  }
  else if (analogRead(CLUTCHPOT) > 1000)
  {
    clutch.write(declutched);
  }
  else if (analogRead(CLUTCHPOT) > 102)
  {
    clutch.write((float) clutched + (((float) declutched - (float)  clutched) * (( (float) analogRead(CLUTCHPOT) - (float) POT_MIN) / ((float) POT_MAX - (float) POT_MIN))));
  }
  else
  {
    clutch.write(clutched);
  }

  if ((digitalRead(UPCHANGE) == 0) && (RELEASE == 0))
  {
    if (started == 0)
    {
      laststart = millis();
      started = 1;
    }
    else if (millis() - laststart > 25)
    {
      clutch.write(declutched);
      delay(100);
      gearchg.attach(GEARCHANGE);
      if (gear > 0)
      {
        gearchg.write(upchange);
      }
      else
      {
        gearchg.write(downchange);
      }
      delay(DELAY);
      gearchg.writeMicroseconds(midpointms);
      delay(150);
      clutch.write(clutched);
      delay(100);
      gear++;
      started = 0;
      RELEASE = 1;
    }
  }
  else if ((digitalRead(DOWNCHANGE) == 0) && (RELEASE == 0))
  {
    if (started == 0)
    {
      laststart = millis();
      started = 1;
    }
    else if (millis() - laststart > 25)
    {
      float clutchpos = declutched;
      float clutchinc = ((float) clutched - (float) declutched) / 40.0;
      clutch.write(declutched);
      delay(100);
      gearchg.attach(GEARCHANGE);
      gearchg.write(downchange);
      delay(DELAY);
      gearchg.writeMicroseconds(midpointms);
      delay(150);
      int startdelay = 10;
      int incdelay = 1;
      while (clutchpos > 0)
      {
        clutchpos += clutchinc;
        if (clutchinc < 0)
        {
          if (clutchpos <= clutched)
          {
            break;
          }
        }
        else if (clutchinc > 0)
        {
          if (clutchpos >= clutched)
          {
            break;
          }
        }
        else
        {
          break;
        }
        clutch.write((int) clutchpos);
        delay(startdelay);
        startdelay += incdelay;
      }
      clutch.write(clutched);
      delay(10);
      gear--;
      started = 0;
      RELEASE = 1;
    }
  }
  else if ((digitalRead(NEUTBUT) == 0) && (RELEASE == 0))
  {
    if (started == 0)
    {
      laststart = millis();
      started = 1;
    }
    else if (millis() - laststart > 50)
    {
      clutch.write(declutched);
      delay(100);
      gearchg.attach(GEARCHANGE);
      gearchg.write(halfup);
      delay(DELAY);
      gearchg.writeMicroseconds(midpointms);
      delay(150);
      clutch.write(clutched);
      delay(100);
      if (gear == 1)
      {
        gear--;
      }
      started = 0;
      RELEASE = 1;
    }
  }
  else if ((digitalRead(DOWNCHANGE)) == 1 && (digitalRead(UPCHANGE) == 1) && (digitalRead(NEUTBUT) == 1))
  {
    RELEASE = 0;
    started = 0;
  }
  else
  {
    gearchg.writeMicroseconds(midpointms);
    delay(10);
  }
}
