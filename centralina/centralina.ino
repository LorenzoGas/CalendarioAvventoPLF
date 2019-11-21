#include <Wire.h>
#include "ds3231.h" //https://github.com/rodan/ds3231

#define BUFF_MAX 256

struct ts t;

//consts
int TIME_START = 17;
int TIME_FINISH = 07;
int NUM_GIORNI = 24;
int ANIMATION_SPEED = 200;

//init vars
int lightSensorState = 0;
int lastLoopDay = 0; //useful to check if the day just changed
int inaugurationAnimationDone = 0;
int nataleAnimationDone = 0;
int randGiorno;

//pins
int pinLightSensor = 2;
int pinContornoOverrideON = A0;
int pinGiorniOverrideON = A2;
int pinNataleOverrideON = A4;
int pinOverrideOFF = A6;

int pinRelayPresepe = 6;
int pinRelayContorno = 7;
int pinRelayNatale = 13;
int pinRelayGiorni[]= {34,35,36,37,38,39,40,41, 42,43,44,45,46,47,48,49, 26,27,28,29,30,31,32,33};

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  DS3231_init(DS3231_CONTROL_INTCN);

  randomSeed(analogRead(0));
  
  //pins in
  pinMode(pinLightSensor, INPUT);
  pinMode(pinContornoOverrideON, INPUT);
  pinMode(pinGiorniOverrideON, INPUT);
  pinMode(pinNataleOverrideON, INPUT);
  pinMode(pinOverrideOFF, INPUT);

  //pins out
  pinMode(pinRelayContorno, OUTPUT);
  digitalWrite(pinRelayContorno, HIGH); //OFF
  pinMode(pinRelayPresepe, OUTPUT);
  digitalWrite(pinRelayPresepe, HIGH); //OFF
  pinMode(pinRelayNatale, OUTPUT);
  digitalWrite(pinRelayNatale, HIGH); //OFF
  for(int i = 0; i < NUM_GIORNI; i++)
  {
    pinMode(pinRelayGiorni[i], OUTPUT); 
    digitalWrite(pinRelayGiorni[i], HIGH); //OFF
  }
}

void loop()
{
  //get date  
  char buff[BUFF_MAX];
  unsigned long now = millis();
  DS3231_get(&t); //Get time with t.year,t.mon, t.mday, t.hour, t.min, t.sec
  
  // display current time
  snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
  Serial.println(buff);
  
  if (digitalRead(pinOverrideOFF) == HIGH)
  {
    turnOffAll();
    Serial.println("pinOverrideOFF");
  }
  else if (digitalRead(pinContornoOverrideON) == HIGH)
  {
    digitalWrite(pinRelayContorno, LOW); //turn on contorno
    Serial.println("pinContornoOverrideON");
  }
  else if (digitalRead(pinGiorniOverrideON) == HIGH)
  {
    animationGiorni(NUM_GIORNI); //animate all days
    Serial.println("pinGiorniOverrideON");
  }
  else if (digitalRead(pinNataleOverrideON) == HIGH)
  {
    turnOnDay(25); //turn on like its christmas
    Serial.println("pinChristmasOverrideON");
  }
  else //no override
  {
    Serial.println("no override");
    if ((t.mon == 12) || ((t.mon == 1) && (t.mday <= 6))) //turn on during Dec and until the 6th of Jan
    {
      lightSensorState = digitalRead(pinLightSensor); //get light sensor state (HIGH = dark)
      if ((lightSensorState == HIGH) || (t.hour >= TIME_START) || (t.hour < TIME_FINISH)) //if its dark or its between TIME_START and TIME_FINISH
      {
        if(t.mon == 12)
          turnOnDay(t.mday); //decenber turns on normal days
        if(t.mon == 1)
          turnOnDay(26); //january turns on only presepe
      }
      else
        turnOffAll();
    }
    else
      turnOffAll();
  }
  Serial.println("-end-");
  lastLoopDay = t.mday; //update old day with the current day
  delay(30000); //wait before updateing the loop
}

void turnOnDay(int num)
{
  if (num == 25) //its Christmas
  {
    //OH OH OH!
    digitalWrite(pinRelayContorno, HIGH); //OFF
    turnOffGiorni();
    
    if(nataleAnimationDone == 0)
    {
      nataleAnimationDone = 1;
      specialAnimation();
      delay(3000);
    }
    digitalWrite(pinRelayNatale, LOW); //ON
    digitalWrite(pinRelayPresepe, LOW); //ON 
    Serial.println("-------!!its Christmas!!-------");
  }
  else if(num >= 26) //its after Christmas
  {
    digitalWrite(pinRelayContorno, HIGH); //OFF
    turnOffGiorni();
    digitalWrite(pinRelayNatale, HIGH); //OFF
    
    digitalWrite(pinRelayPresepe, LOW); //ON
    Serial.println("-------!!its after Christmas!!-------");
  }
  else if(num == 1) //its the 1st, turn on at 18:00 for inauguration
  { 
    digitalWrite(pinRelayPresepe, HIGH); //OFF
    digitalWrite(pinRelayNatale, HIGH); //OFF

    if(t.hour >= 18)
    {
      if(inaugurationAnimationDone == 0)
      {
        Serial.println("-------!!inauguration!!-------");
        inaugurationAnimationDone = 1;
        Serial.println(inaugurationAnimationDone);
        specialAnimation();
        //turn on only 1 and contorno
        delay(3000);
        digitalWrite(pinRelayContorno, LOW); //ON
        digitalWrite(pinRelayGiorni[0], LOW);//ON
        for(int i = 1; i < NUM_GIORNI; i++)
        {
          digitalWrite(pinRelayGiorni[i], HIGH);//OFF
        }
      }
      else
      {
        Serial.println("inauguration was done before");
        digitalWrite(pinRelayContorno, LOW); //ON
        digitalWrite(pinRelayGiorni[0], LOW);//ON
        for(int i = 1; i < NUM_GIORNI; i++)
        {
          digitalWrite(pinRelayGiorni[i], HIGH);//OFF
        }
      }
    }
    Serial.println("-------!!its the 1st!!-------");
  }
  else
  {
    digitalWrite(pinRelayPresepe, HIGH); //OFF
    digitalWrite(pinRelayNatale, HIGH); //OFF
    
    digitalWrite(pinRelayContorno, LOW); //ON    
    if (num > lastLoopDay) //the day just changed 
    {
      animationGiorni(num);
      Serial.println("the day just changed");
    }
    else
    {
      for(int i = 0; i < NUM_GIORNI; i++)
      {
        if(i < num) //array starts at index 0 
          digitalWrite(pinRelayGiorni[i], LOW);//ON
        else
          digitalWrite(pinRelayGiorni[i], HIGH);//OFF
      }
    }
    Serial.println("the day on is:");
    Serial.println(num);
  }

}

void animationGiorni(int num)
{
  turnOffGiorni();
  for(int i = 0; (i < num) && (i < NUM_GIORNI); i++) //progressively turn on days
  {
    digitalWrite(pinRelayGiorni[i], LOW); //ON
    delay(ANIMATION_SPEED);
  }
}

void specialAnimation()
{
  for(int i = 1; i <= 5; i++) //crescendo
  {
    turnOnAllGiorni();
    digitalWrite(pinRelayContorno, LOW); //ON
    delay(3000/i); //stay on for mills
    turnOffAll();
    delay(1500/i); //stay off for mills
  }
  Serial.println("end crescendo");
  for(int i = 0; i < 20; i++) //random
  {
    randGiorno = random(NUM_GIORNI);
    Serial.println(randGiorno);
    digitalWrite(pinRelayGiorni[randGiorno], LOW);//ON
    delay(500-(i*5)); //stay on for mills
    digitalWrite(pinRelayGiorni[randGiorno], HIGH);//OFF
    delay(200-(i*5)); //stay off for mills
  }
  Serial.println("end random"); 
}

void turnOffAll()
{
  digitalWrite(pinRelayContorno, HIGH); //OFF
  digitalWrite(pinRelayPresepe, HIGH); //OFF
  digitalWrite(pinRelayNatale, HIGH); //OFF
  turnOffGiorni();
}

void turnOffGiorni()
{
  for(int i = 0; i < NUM_GIORNI; i++)
    digitalWrite(pinRelayGiorni[i], HIGH); //OFF
}

void turnOnAllGiorni()
{
  for(int i = 0; i < NUM_GIORNI; i++)
    digitalWrite(pinRelayGiorni[i], LOW); //ON
}
