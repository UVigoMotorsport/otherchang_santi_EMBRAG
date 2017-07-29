#include <Servo.h>

#define UP 1
#define DOWN 2
#define NEUT 3

#define GEARCHANGE 9
#define CLUTCH 8
#define GEARPOS A0
#define CLUTCHPOS A1
#define CLUTCHPOT A2
#define CLUTCHBUT 4
#define NEUTBUT A3
#define NEUTRAL 5
#define ECUCUT 13

#define TESTOUT 11

#define POT_MAX 1023
#define POT_MIN 0

#define UPCHANGE 2
#define DOWNCHANGE 3

#define DELAY 600

#define CUTUP 0
#define CUTUPDELAY 250
int cutup = 0;

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

#define MINPRESS 25
#define CLUTCHSTEPS 30
#define CLUTCHINITDELAY 9
#define CLUTCHINCDELAY 3
#define CLUTCHFULLTIME 200

Servo gearchg;
Servo clutch;

#define ERRORMODE 1
#define NORMALMODE 2

char gears[7] = {'N', '1', '2', '3', '4', '5', '6'};
int gear = 0;

int MODE = ERRORMODE;

unsigned long statestart = 0;
unsigned long lastmode = 0;
unsigned long clutchtime = 0;
unsigned long laststart = 0;
unsigned long clutchwait = 0;
unsigned long lastshow = 0;
int modestarted = 0;
int forcedeclutched = 0;
int change = 0;
int started = 0;

int CHANGESTATE = 0;

float clutchpos = declutched;
float clutchinc = ((float) clutched - (float) declutched) / (float) CLUTCHSTEPS;

int RELEASE = 0;
void setup()
{
  pinMode(GEARCHANGE, OUTPUT);
  pinMode(CLUTCH, OUTPUT);
  pinMode(UPCHANGE, INPUT);
  pinMode(DOWNCHANGE, INPUT);
  pinMode(NEUTBUT, INPUT);
  pinMode(CLUTCHBUT, INPUT);
  pinMode(ECUCUT, OUTPUT);
  pinMode(TESTOUT, OUTPUT);


  gearchg.attach(GEARCHANGE);
  clutch.attach(CLUTCH);
  gearchg.writeMicroseconds(midpointms);
  clutch.write(clutched);
  digitalWrite(ECUCUT, 0);
  digitalWrite(TESTOUT, 0);
  Serial.begin(9600);
}

void loop()
{
  if (MODE == ERRORMODE)
  {
    digitalWrite(LED_BUILTIN, 0);
    if (digitalRead(NEUTRAL) == 0)
    {
      if (!modestarted)
      {
        lastmode = millis();
        modestarted = 1;
      }
      if (millis() - lastmode > MINPRESS)
      {
        digitalWrite(LED_BUILTIN, 1);
        MODE = NORMALMODE;
        gear = 0;
      }
    }
    else
    {
      modestarted = 0;
    }
  }

  if (MODE == NORMALMODE)
  {
    if (gear != 0 && digitalRead(NEUTRAL) == 0)
    {
      MODE = ERRORMODE;
    }
  }

  if (CHANGESTATE == 0)
  {

    if (digitalRead(CLUTCHBUT) == 0)
    {
      clutch.write(declutched);
      if (CHANGESTATE == 0)
      {
        forcedeclutched = 1;
      }
    }
    else if (analogRead(CLUTCHPOT) > 1000)
    {
      clutch.write(declutched);
    }
    else if (analogRead(CLUTCHPOT) > 102  && !forcedeclutched)
    {
      clutch.write((float) clutched + (((float) declutched - (float)  clutched) * (( (float) analogRead(CLUTCHPOT) - (float) POT_MIN) / ((float) POT_MAX - (float) POT_MIN))));
    }
    else
    {
      clutch.write(clutched);
      forcedeclutched = 0;
    }

    if ((digitalRead(UPCHANGE) == 0) && (RELEASE == 0))
    {
      if (started == 0)
      {
        laststart = millis();
        started = 1;
      }
      else if (millis() - laststart > MINPRESS)
      {
        started = 0;
        change = UP;
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
      else if (millis() - laststart > MINPRESS)
      {
        started = 0;
        change = DOWN;
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
      else if (millis() - laststart > MINPRESS)
      {
        started = 0;
        change = NEUT;
        RELEASE = 1;
      }
    }
  }
  if ((digitalRead(DOWNCHANGE)) == 1 && (digitalRead(UPCHANGE) == 1) && (digitalRead(NEUTBUT) == 1))
  {
    RELEASE = 0;
    started = 0;
  }
  if (change != 0 && CHANGESTATE == 0)
  {
    if (MODE == NORMALMODE)
    {
      if (gear <= 1 && change == DOWN)
      {
        change = 0;
      }
      else if (gear == 6 && change == UP)
      {
        change = 0;
      }
      else if (gear != 1 && change == NEUT)
      {
        change = 0;
      }
      else
      {
        if (CUTUP && change == UP)
        {
          cutup = 1;
        }
        else
        {
          cutup = 0;
        }
        CHANGESTATE++;
        statestart = millis();
      }
    }
    else
    {
      CHANGESTATE++;
      statestart = millis();
    }
  }

  if (CHANGESTATE == 1)
  {
    if (!forcedeclutched && !cutup)
    {
      clutch.write(declutched);
      if (millis() - statestart > CLUTCHFULLTIME)
      {
        CHANGESTATE++;
        statestart = millis();
      }
    }
    else
    {
      if (cutup)
      {
        digitalWrite(ECUCUT, 1);
      }
      CHANGESTATE++;
      statestart = millis();
    }
  }

  if (CHANGESTATE == 2)
  {
    if (gear == 0 && change == UP && MODE == NORMALMODE)
    {
      gearchg.write(downchange);
    }
    else
    {
      if (change == UP)
      {
        gearchg.write(upchange);
      }
      else if (change == DOWN)
      {
        gearchg.write(downchange);
      }
      else if (change == NEUT)
      {
        gearchg.write(halfup);
      }
      else
      {
        change = 0;
        CHANGESTATE = 0;
      }
    }

    if (millis() - statestart > DELAY)
    {
      CHANGESTATE++;
      statestart = millis();
    }
  }

  if (CHANGESTATE == 3)
  {
    gearchg.write(midpointms);
    if (millis() - statestart > 200)
    {
      CHANGESTATE++;
      statestart = millis();
    }
  }

  if (CHANGESTATE == 4)
  {
    if (MODE == NORMALMODE)
    {
      if ((gear == 0 && change == UP) || (gear == 1 && change == UP) || (gear == 2 && change == DOWN))
      {
        digitalWrite(TESTOUT, 1);
        if (digitalRead(NEUTRAL) == 0)
        {
          MODE = ERRORMODE;
        }
      }
      else if (gear == 1 && change == NEUT)
      {
        digitalWrite(TESTOUT, 0);
        if (digitalRead(NEUTRAL) == 1)
        {
          MODE = ERRORMODE;
        }
      }
    }
    clutchpos = declutched;
    clutchtime = millis();
    clutchwait = CLUTCHINITDELAY;
    CHANGESTATE++;
    statestart = millis();
  }

  if (CHANGESTATE == 5)
  {
    if (cutup || forcedeclutched)
    {
      digitalWrite(ECUCUT, 0);
    }
    else
    {
      if (change == DOWN)
      {
        if (millis() - clutchtime > clutchwait)
        {
          clutch.write((int) clutchpos);
          clutchpos += clutchinc;
          clutchwait += CLUTCHINCDELAY;
          clutchtime = millis();
        }
        if (clutchinc < 0)
        {
          if (clutchpos <= clutched)
          {
            clutch.write(clutched);
            CHANGESTATE++;
            statestart = millis();
          }
        }
        else if (clutchinc > 0)
        {
          if (clutchpos >= clutched)
          {
            clutch.write(clutched);
            CHANGESTATE++;
            statestart = millis();
          }
        }
        else
        {
          clutch.write(clutched);
          CHANGESTATE++;
          statestart = millis();
        }
      }
      else
      {
        clutch.write(clutched);
        if (millis() - statestart > CLUTCHFULLTIME)
        {
          CHANGESTATE++;
          statestart = millis();
        }
      }
    }
  }

  if (CHANGESTATE == 6)
  {
    if (MODE == NORMALMODE)
    {
      if (change == UP)
      {
        gear++;
      }
      else if (change == DOWN)
      {
        gear--;
      }
      else if (change == NEUT)
      {
        gear = 0;
      }
    }
    CHANGESTATE = 0;
    change = 0;
  }

  if (millis() - lastshow > 600)
  {
    Serial.println(gears[gear]);
    lastshow = millis();
  }
}
