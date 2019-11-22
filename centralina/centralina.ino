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
int lastLoopDay = 0; //useful to check if the day just changed
bool inaugurationAnimationDone = false;
bool nataleAnimationDone = false;
int randGiorno;

//pins
int pinContornoOverrideON = A0;
int pinGiorniOverrideON = A2;
int pinNataleOverrideON = A4;

int pinRelayPresepe = 6;
int pinsRelaysContorno[] = {8,9,10}; //TODO NUMERI GIORNI
int pinRelayNatale = 13;
int pinsRelaysGiorni[]= {34,35,36,37,38,39,40,41, 42,43,44,45,46,47,48,49, 26,27,28,29,30,31,32,33};

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  DS3231_init(DS3231_CONTROL_INTCN);

  randomSeed(analogRead(0));
  
  //pins in
  pinMode(pinContornoOverrideON, INPUT);
  pinMode(pinGiorniOverrideON, INPUT);
  pinMode(pinNataleOverrideON, INPUT);

  //pins out
  pinMode(pinRelayPresepe, OUTPUT);
  digitalWrite(pinRelayPresepe, HIGH); //OFF
  pinMode(pinRelayNatale, OUTPUT);
  digitalWrite(pinRelayNatale, HIGH); //OFF
  for(int pin : pinsRelaysContorno)
  {
    pinMode(pin, OUTPUT); 
    digitalWrite(pin, HIGH); //OFF
  }
  for(int pin : pinsRelaysGiorni)
  {
    pinMode(pin, OUTPUT); 
    digitalWrite(pin, HIGH); //OFF
  }
}

void loop()
{
  //get date  
  char buff[BUFF_MAX];
  unsigned long now = millis();
  DS3231_get(&t); //Get time with t.year,t.mon, t.mday, t.hour, t.min, t.sec
  
  // display current time
  snprintf(buff, BUFF_MAX, "----- %d.%02d.%02d %02d:%02d:%02d -----", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
  Serial.println(buff);
  
  if (digitalRead(pinContornoOverrideON) == HIGH)
  {
    turnOnContorno();
    Serial.println("Contorno Override ON");
  }
  else if (digitalRead(pinGiorniOverrideON) == HIGH)
  {
    animationGiorni(NUM_GIORNI); //animate all days
    Serial.println("Giorni Override ON");
  }
  else if (digitalRead(pinNataleOverrideON) == HIGH)
  {
    turnOnDay(25); //turn on like its christmas
    Serial.println("Christmas Override ON");
  }
  else //no override
  {
    Serial.println("no overrides");
    if ((t.mon == 12) || ((t.mon == 1) && (t.mday <= 6))) //turn on during Dec and until the 6th of Jan
    {
      if ((t.hour >= TIME_START) || (t.hour < TIME_FINISH)) //if its between TIME_START and TIME_FINISH
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
  delay(10000); //wait before updateing the loop
}

void turnOnDay(int num)
{
  if (num == 25) //its Christmas
  {
    //OH OH OH!
    turnOffContorno();
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
    turnOffContorno();
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
      if(!inaugurationAnimationDone)
      {
        Serial.println("-------!!inauguration!!-------");
        inaugurationAnimationDone = true;
        Serial.println(inaugurationAnimationDone);
        specialAnimation();
        //turn on only 1 and contorno
        delay(3000);
        turnOnContorno();
        digitalWrite(pinsRelaysGiorni[0], LOW);//ON
        for(int i = 1; i < NUM_GIORNI; i++)
        {
          digitalWrite(pinsRelaysGiorni[i], HIGH);//OFF
        }
      }
      else
      {
        Serial.println("inauguration was done before");
        turnOnContorno();
        digitalWrite(pinsRelaysGiorni[0], LOW);//ON
        for(int i = 1; i < NUM_GIORNI; i++)
        {
          digitalWrite(pinsRelaysGiorni[i], HIGH);//OFF
        }
      }
    }
    else{
      turnOffContorno();
      turnOffGiorni();
    }
    Serial.println("-------!!its the 1st!!-------");
  }
  else
  {
    digitalWrite(pinRelayPresepe, HIGH); //OFF
    digitalWrite(pinRelayNatale, HIGH); //OFF
    
    turnOnContorno();    
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
          digitalWrite(pinsRelaysGiorni[i], LOW);//ON
        else
          digitalWrite(pinsRelaysGiorni[i], HIGH);//OFF
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
    digitalWrite(pinsRelaysGiorni[i], LOW); //ON
    delay(ANIMATION_SPEED);
  }
}

void specialAnimation()
{
  for(int i = 1; i <= 5; i++) //crescendo
  {
    turnOnAllGiorni();
    turnOnContorno();
    delay(3000/i); //stay on for mills
    turnOffAll();
    delay(1500/i); //stay off for mills
  }
  Serial.println("end crescendo");
  for(int i = 0; i < 20; i++) //random
  {
    randGiorno = random(NUM_GIORNI);
    Serial.println(randGiorno);
    digitalWrite(pinsRelaysGiorni[randGiorno], LOW);//ON
    delay(500-(i*5)); //stay on for mills
    digitalWrite(pinsRelaysGiorni[randGiorno], HIGH);//OFF
    delay(200-(i*5)); //stay off for mills
  }
  Serial.println("end random"); 
}

void turnOnContorno(){
  for(int pin : pinsRelaysContorno)
    digitalWrite(pin, LOW); //ON
}

void turnOffContorno(){
  for(int pin : pinsRelaysContorno)
    digitalWrite(pin, HIGH); //OFF
}

void turnOffAll()
{
  turnOffContorno();
  digitalWrite(pinRelayPresepe, HIGH); //OFF
  digitalWrite(pinRelayNatale, HIGH); //OFF
  turnOffGiorni();
}

void turnOffGiorni()
{
  for(int i = 0; i < NUM_GIORNI; i++)
    digitalWrite(pinsRelaysGiorni[i], HIGH); //OFF
}

void turnOnAllGiorni()
{
  for(int i = 0; i < NUM_GIORNI; i++)
    digitalWrite(pinsRelaysGiorni[i], LOW); //ON
}
