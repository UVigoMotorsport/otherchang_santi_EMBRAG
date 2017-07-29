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
#define halfup 65
#define downchange 180
#define upchange 0
#define movetime 500

#define clutched 0
#define declutched 100

#define MINPRESS 15

#define CLUTCHDELAY 3

Servo gearchg;
Servo clutch;

#define ERRORMODE 1
#define NORMALMODE 2

char gears[7] = {'N', '1', '2', '3', '4', '5', '6'};
int gear = 0;

int MODE = ERRORMODE;

unsigned long lastmove = 0;
int currmove = 0;

int change = 0;
int changeinprogress = 0;
unsigned long lastshow = 0;
int nowclutched = 0;
unsigned long lastmode = 0;
int modestarted = 0;
unsigned long changestart = 0;
unsigned long laststart = 0;
int started = 0;
int forcedeclutched = 0;

float nowclutchpos = 0;

int RELEASE = 0;

int CHANGESTATE = 0;

void setup()
{
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
  if (MODE == ERRORMODE)
  {
    if (digitalRead(NEUTRAL) == 0)
    {
      if (!modestarted)
      {
        lastmode = millis();
        modestarted = 1;
      }
      if (millis() - lastmode > MINPRESS)
      {
        MODE = NORMALMODE;
        gear = 0;
      }
    }
    else
    {
      modestarted = 0;
    }
  }

  if (digitalRead(CLUTCHBUT) == 0)
  {
    clutch.write(declutched);
    forcedeclutched = 1;
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

  if (!changeinprogress)
  {
    if ((digitalRead(UPCHANGE) == 0) && (RELEASE == 0))
    {
      if (started == 0)
      {
        laststart = millis();
        started = UP;
      }
      else if (started != UP)
      {
        started = 0;
      }
    }
    if ((digitalRead(DOWNCHANGE) == 0) && (RELEASE == 0))
    {
      if (started == 0)
      {
        laststart = millis();
        started = DOWN;
      }
      else if (started != DOWN)
      {
        started = 0;
      }
    }
    if ((digitalRead(NEUTBUT) == 0) && (RELEASE == 0))
    {
      if (started == 0)
      {
        laststart = millis();
        started = NEUT;
      }
      else if (started != NEUT)
      {
        started = 0;
      }
    }
    if ((digitalRead(DOWNCHANGE)) == 1 && (digitalRead(UPCHANGE) == 1) && (digitalRead(NEUTBUT) == 1))
    {
      if (millis() - laststart < MINPRESS)
      {
        started = 0;
      }
      RELEASE = 0;
    }

    if (started && (millis() - laststart > MINPRESS))
    {
      changeinprogress = 1;
      changestart = millis();
    }
  }

  cutup = 0;
  if (changeinprogress)
  {
    int stat = 1;
    if (MODE == ERRORMODE)
    {
      stat = gearchange(started);
      if (stat == 0)
      {
        changeinprogress = 0;
        started = 0;
      }
    }
    else if (MODE == NORMALMODE)
    {
      if (gear == 0)
      {
        if (started == UP)
        {
          stat = gearchange(DOWN);
          if (stat == 0)
          {
            changeinprogress = 0;
            if (digitalRead(NEUTRAL) == 1)
            {
              gear++;
            }
            else
            {
              MODE = ERRORMODE;
            }
          }
        }
        else
        {
          changeinprogress = 0;
        }
      }
      else if (gear == 1)
      {
        if (started == UP)
        {
          if (CUTUP)
          {
            cutup = 1;
          }
          stat = gearchange(UP);
          if (stat == 0)
          {
            changeinprogress = 0;
            if (digitalRead(NEUTRAL) == 1)
            {
              gear++;
            }
            else
            {
              MODE = ERRORMODE;
            }
          }
        }
        else if (started == NEUT)
        {
          stat = gearchange(NEUT);
          if (stat == 0)
          {
            changeinprogress = 0;
            if (digitalRead(NEUTRAL) == 0)
            {
              gear--;
            }
            else
            {
              MODE = ERRORMODE;
            }
          }
        }
        else
        {
          changeinprogress = 0;
        }
      }
      else if (gear > 1)
      {
        if (started == UP || started == DOWN)
        {
          if (CUTUP && started == UP)
          {
            cutup = 1;
          }
          stat = gearchange(started);
          if (stat == 0)
          {
            changeinprogress = 0;
            if (digitalRead(NEUTRAL) == 1)
            {
              if (started == UP)
              {
                gear++;
              }
              else if (started == DOWN)
              {
                gear--;
              }
            }
            else
            {
              MODE = ERRORMODE;
            }
          }
        }
        else
        {
          changeinprogress = 0;
        }
      }
    }
  }
}

int gearchange(int dir)
{
  if (CHANGESTATE == 0)
  {
    if (cutup)
    {
      CHANGESTATE++;
      changestart = millis();
      return -1;
    }
    clutch.write(declutched);
    if (millis() - changestart > 100 || forcedeclutched)
    {
      CHANGESTATE++;
      changestart = millis();
    }
    return -1;
  }
  else if (CHANGESTATE == 1)
  {
    int dirchange = midpointms;
    if (dir == UP)
    {
      dirchange = upchange;
    }
    else if (dir == DOWN)
    {
      dirchange = downchange;
    }
    else if (dir == NEUT)
    {
      dirchange = halfup;
    }
    gearchg.write(dirchange);
    int delaychange = DELAY;
    if(cutup)
    {
      delaychange = CUTUPDELAY;
    }
    if (millis() - changestart > delaychange)
    {
      CHANGESTATE++;
      changestart = millis();
    }
    return -1;
  }
  else if (CHANGESTATE == 2)
  {
    gearchg.writeMicroseconds(midpointms);
    if (millis() - changestart > 150)
    {
      CHANGESTATE++;
      changestart = millis();
      nowclutchpos = declutched;
    }
    return -1;
  }
  else if (CHANGESTATE == 3)
  {
    if (forcedeclutched || cutup)
    {
      CHANGESTATE++;
      changestart = millis();
    }
    else
    {
      if (dir == DOWN)
      {
        float clutchinc = ((float) clutched - (float) declutched) / 10.0;
        clutch.write((int) nowclutchpos);
        if (millis() - changestart > CLUTCHDELAY)
        {
          changestart = millis();
          nowclutchpos += clutchinc;
          return -1;
        }
        if (abs(nowclutchpos - (float) clutched) < clutchinc)
        {
          nowclutchpos = clutched;
          clutch.write(clutched);
          CHANGESTATE++;
          changestart = millis();
          return -1;
        }
      }
      else
      {
        clutch.write(clutched);
        if (millis() - changestart > 100)
        {
          CHANGESTATE++;
          changestart = millis();
        }
      }
    }
    return -1;
  }
  if (CHANGESTATE == 4)
  {
    gearchg.writeMicroseconds((int)midpointms);
    if (!forcedeclutched || !cutup)
    {
      clutch.write(clutched);
    }
    return 0;
  }
}
