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
#define midpointpos 0
#define halfup 65
//#define halfup 1330
#define halfuppos 0
#define downchange 180
//#define downchange 2100
#define downchangepos 0
#define upchange 0
//#define upchange 900
#define upchangepos 0

#define clutched 0
//#define clutched 900
#define clutchedpos 0
#define declutched 100
//#define declutched 2100
#define declutchedpos 0

#define MINPRESS 25
#define CLUTCHSTEPS 40
#define CLUTCHINITDELAY 5
#define CLUTCHINCDELAY 1
#define CLUTCHFULLTIME 200

#define POTFLUFF 2

#define TIMED 1
#define POTS 2
int SERVOMODE = TIMED;


Servo gearchg;
Servo clutch;

#define MAXLEVERTIME 700

#define ERRORMODE 1
#define NORMALMODE 2

#define CONTINUE 0

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
int realgeardir = 0;

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
  }
  else if (MODE == NORMALMODE)
  {
    digitalWrite(LED_BUILTIN, 1);
  }

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

  if (MODE == NORMALMODE)
  {
    if (gear != 0 && digitalRead(NEUTRAL) == 0)
    {
      MODE = ERRORMODE;
    }
  }

  if (CHANGESTATE == 0)
  {
    gearchg.write(midpointms);
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
      if (SERVOMODE == POTS)
      {
        int clutchpotpos = analogRead(CLUTCHPOS);

        if (millis() - statestart > MAXLEVERTIME)
        {
          MODE = ERRORMODE;
          if (CONTINUE)
          {
            CHANGESTATE++;
          }
          else
          {
            CHANGESTATE = 0;
            change = 0;
          }
          statestart = millis();
        }
        else
        {
          if (declutchedpos > clutchedpos)
          {
            if (clutchpotpos >= (declutchedpos - POTFLUFF))
            {
              CHANGESTATE++;
              statestart = millis();
            }
          }
          else if (declutchedpos < clutchedpos)
          {
            if (clutchpotpos <= (declutchedpos + POTFLUFF))
            {
              CHANGESTATE++;
              statestart = millis();
            }
          }
        }
      }
      else if (millis() - statestart > CLUTCHFULLTIME)
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
      realgeardir = DOWN;
    }
    else
    {
      if (change == UP)
      {
        gearchg.write(upchange);
        realgeardir = UP;
      }
      else if (change == DOWN)
      {
        gearchg.write(downchange);
        realgeardir = DOWN;
      }
      else if (change == NEUT)
      {
        gearchg.write(halfup);
        digitalWrite(TESTOUT, 0);
        realgeardir = NEUT;
      }
      else
      {
        change = 0;
        CHANGESTATE = 0;
      }
    }

    if (digitalRead(NEUTRAL) == 0)
    {
      MODE = NORMALMODE;
      if(realgeardir == UP || realgeardir == NEUT)
      {
        gear = 1;
      }
      else if(realgeardir == DOWN)
      {
        gear = 2;
      }
    }

    if (SERVOMODE == POTS)
    {
      int gearpos = analogRead(GEARPOS);
      int newpos = midpointpos;
      if (realgeardir == NEUT)
      {
        newpos = halfuppos;
      }
      else if (realgeardir == UP)
      {
        newpos = upchangepos;
      }
      else if (realgeardir == DOWN)
      {
        newpos = downchangepos;
      }

      if (millis() - statestart > MAXLEVERTIME)
      {
        MODE = ERRORMODE;
        CHANGESTATE = 0;
        change = 0;
        statestart = millis();
      }
      else
      {
        if (newpos > midpointpos)
        {
          if (gearpos >= (newpos - POTFLUFF))
          {
            CHANGESTATE++;
            statestart = millis();
          }
        }
        else if (newpos < midpointpos)
        {
          if (gearpos <= (newpos + POTFLUFF))
          {
            CHANGESTATE++;
            statestart = millis();
          }
        }
      }
    }
    else if (millis() - statestart > DELAY)
    {
      CHANGESTATE++;
      statestart = millis();
    }
  }

  if (CHANGESTATE == 3)
  {
    gearchg.write(midpointms);
    if (SERVOMODE == POTS)
    {
      int gearpos = analogRead(GEARPOS);
      int oldpos = midpointpos;
      if (realgeardir == NEUT)
      {
        oldpos = halfuppos;
      }
      else if (realgeardir == UP)
      {
        oldpos = upchangepos;
      }
      else if (realgeardir == DOWN)
      {
        oldpos = downchangepos;
      }

      if (millis() - statestart > MAXLEVERTIME)
      {
        MODE = ERRORMODE;
        CHANGESTATE = 0;
        change = 0;
        statestart = millis();
      }
      else
      {
        if (midpointpos > oldpos)
        {
          if (gearpos >= (midpointpos - POTFLUFF))
          {
            CHANGESTATE++;
            statestart = millis();
          }
        }
        else if (midpointpos < oldpos)
        {
          if (gearpos <= (midpointpos + POTFLUFF))
          {
            CHANGESTATE++;
            statestart = millis();
          }
        }
      }
    }
    else if (millis() - statestart > 200)
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
      CHANGESTATE++;
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
        if (SERVOMODE == POTS)
        {
          int clutchpotpos = analogRead(CLUTCHPOS);
          if (millis() - statestart > MAXLEVERTIME)
          {
            MODE = ERRORMODE;
            CHANGESTATE++;
            statestart = millis();
          }
          else
          {
            if (clutchedpos > declutchedpos)
            {
              if (clutchpotpos >= (clutchedpos - POTFLUFF))
              {
                CHANGESTATE++;
                statestart = millis();
              }
            }
            else if (clutchedpos < declutchedpos)
            {
              if (clutchpotpos <= (clutchedpos + POTFLUFF))
              {
                CHANGESTATE++;
                statestart = millis();
              }
            }
          }
        }
        else if (millis() - statestart > CLUTCHFULLTIME)
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
    if (NORMALMODE)
    {
      Serial.println(gears[gear]);
    }
    else
    {
      Serial.println("X");
    }
    if (Serial.read() == 't')
    {
      while (true)
      {
        int in = Serial.read();
        if (in == 'u')
        {
          gearchg.write(halfup);
          delay(MAXLEVERTIME);
          Serial.println(analogRead(GEARPOS));
        }
        else if (in == '+')
        {
          gearchg.write(upchange);
          delay(MAXLEVERTIME);
          Serial.println(analogRead(GEARPOS));
        }
        else if (in == '-')
        {
          gearchg.write(downchange);
          delay(MAXLEVERTIME);
          Serial.println(analogRead(GEARPOS));
        }
        else if (in == 'm')
        {
          gearchg.write(midpointms);
          delay(MAXLEVERTIME);
          Serial.println(analogRead(GEARPOS));
        }
        else if (in == 'c')
        {
          clutch.write(clutched);
          delay(MAXLEVERTIME);
          Serial.println(analogRead(CLUTCHPOS));
        }
        else if (in == 'd')
        {
          clutch.write(declutched);
          delay(MAXLEVERTIME);
          Serial.println(analogRead(CLUTCHPOS));
        }
        else if (in == 'q')
        {
          break;
        }
      }
    }
    lastshow = millis();
  }
}
