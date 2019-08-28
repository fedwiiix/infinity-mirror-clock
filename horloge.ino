#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
tmElements_t tm;
#include <PololuLedStrip.h>

#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

char IN[4]={A0,A1,A2,A3};
int i,j,k;
int heure,sec,lHour,lSecond,lMinute;
int decalage= 23;   // 23 ou 31
int value;
int autoreglage=0;
int lumvalue = 200;

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<11> ledStrip;
// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 60
rgb_color colors[LED_COUNT];

unsigned int loopCount = 0;
unsigned const int maxLoop = 200;  // number of ring
boolean showAnimation = true;

/**********************/
/* pour l affichage   */
/**********************/
void clockCircle()
{
  tmElements_t tm;
  if (RTC.read(tm)) {

    colors[lHour] = (rgb_color){ 0, 0, 0};      // reset led
    colors[lMinute] = (rgb_color){ 0, 0, 0};
    colors[lSecond] = (rgb_color){ 0, 0, 0};
    
    
    colors[-1+decalage] = (rgb_color){ 4, 3, 3};    // affichage des 4 led de directions
    colors[14+decalage] = (rgb_color){ 4, 3, 3};
    if(29+decalage>59)
      colors[29+decalage-60] = (rgb_color){ 4, 3, 3};
    else
      colors[29+decalage] = (rgb_color){ 4, 3, 3};
    if(44+decalage>59)
      colors[44+decalage-60] = (rgb_color){ 4, 3, 3};
    else
      colors[44+decalage] = (rgb_color){ 4, 3, 3};

    lHour = decalage + 59 - (tm.Hour%12) * 60 / 12 ;    // calibrage de l'heure
    lMinute = decalage + 59 - tm.Minute;
    lSecond = decalage + 59 - tm.Second;

    if( lHour>59)         // calibrage en fonction du decalage
      lHour=lHour-60;
     if( lSecond>59)
      lSecond=lSecond-60;
     if( lMinute>60)
      lMinute=lMinute-60;

    colors[lHour] = (rgb_color){ lumvalue + (lumvalue/5) , 0, 0};      // affichage led
    colors[lMinute] = (rgb_color){ lumvalue, lumvalue, 0};
    colors[lSecond] = (rgb_color){ 0, 0, lumvalue};

    
    if(lHour == lMinute)                          // si deux leds se chevauchent
      colors[lHour] = (rgb_color){ lumvalue, (lumvalue/5)+14, 0};
    if(lSecond == lHour)
      colors[lHour] = (rgb_color){ lumvalue, 0, lumvalue};
    if(lSecond == lMinute)
      colors[lSecond] = (rgb_color){ 0, lumvalue, lumvalue};

    if (tm.Minute == 0 && tm.Second == 0 && tm.Hour>8 && tm.Hour<23 ) {
      showAnimation = true;
      lumvalue = 200;
    }else if (tm.Minute == 30 && tm.Second == 0 && tm.Hour==22 ) {
      lumvalue=1;
      autoreglage++;
      if( autoreglage > 5 ) {
        setTime(tm.Hour,tm.Minute-1,tm.Second,0,0,0);
        RTC.set(now());
        autoreglage=0;
      }
    }
    
  }
}

/**********************/
/* For animations     */
/**********************/
void fade(unsigned char *val, unsigned char fadeTime)
{
  if (*val != 0)
  {
    unsigned char subAmt = *val >> fadeTime;  // val * 2^-fadeTime
    if (subAmt < 1)
      subAmt = 1;  // make sure we always decrease by at least 1
    *val -= subAmt;  // decrease value of byte pointed to by val
  }
}
void traditionalColors()
{
  unsigned char cycle = 0; // Timing speed
    cycle = millis() >> 5;
  unsigned const int extendedLEDCount = (((LED_COUNT-1)/30)+1)*30;
  for (int i = 0; i < extendedLEDCount; i++)
  {
    unsigned int idx = (i + cycle)%extendedLEDCount;
    if (idx < LED_COUNT)  // if our transformed index exists
    {
      if (i % 12 == 0)
      {
        switch ((i/12)%5)
        {
           case 0:  // red
             colors[idx].red = 200; 
             colors[idx].green = 10; 
             colors[idx].blue = 10;  
             break;
           case 1:  // green
             colors[idx].red = 10; 
             colors[idx].green = 200;  
             colors[idx].blue = 10; 
             break;
           case 2:  // orange
             colors[idx].red = 200;  
             colors[idx].green = 120; 
             colors[idx].blue = 0; 
             break;
           case 3:  // blue
             colors[idx].red = 10; 
             colors[idx].green = 10; 
             colors[idx].blue = 200; 
             break;
           case 4:  // magenta
             colors[idx].red = 200; 
             colors[idx].green = 64;  
             colors[idx].blue = 145;  
             break;
        }
      }
      else
      {
        // fade the 3/4 of LEDs that we are not currently brightening
        fade(&colors[idx].red, 4);
        fade(&colors[idx].green, 4);
        fade(&colors[idx].blue, 4);
      }
    }
  }
}
void setup()
{
  for(i=3;i<10;i++)
    pinMode(i, OUTPUT); 
  for(i=0;i<4;i++)
    pinMode(IN[i], OUTPUT);
   pinMode(10, OUTPUT); 
   digitalWrite(10, HIGH); 

  for (int i = 0; i < LED_COUNT; i++)
  {
    colors[i] = (rgb_color){0, 0, 0};
  }
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2
}
void loop()
{

  if (showAnimation) {
    traditionalColors();
    ledStrip.write(colors, LED_COUNT); 
    loopCount++;
    if (loopCount > maxLoop) {
      loopCount = 0; 
      showAnimation = false;
      for(int t = 0; t < LED_COUNT; t++)
        colors[t] = (rgb_color){ 0, 0, 0};
    }
  } else {

    if(sec != tm.Second){
      clockCircle();
      ledStrip.write(colors, LED_COUNT); 
      sec = tm.Second;
    }
  }    
  
  ///////////////////////////////////////////////////////////////////
   
  RTC.read(tm);
  heure=tm.Hour *100 + tm.Minute;//minute second

for(i=3;i>=0;i--){
  for(j=0;j<4;j++)
    digitalWrite(IN[j], 1);
   for(j=3;j<10;j++)
    digitalWrite(j, 1);
  digitalWrite(IN[i], 0);

  switch(i){
      case 0:        chiffre(heure%10); 
      break;
      case 1:        chiffre(heure/10 %10);
      break;
      case 2:        chiffre(heure/100 %10);
      break;
      case 3:  
            if( heure/1000 %10 != 0)     
              chiffre(heure/1000 %10);
      break;
 }
 delay(5);
}

// ******************************************* bp poussoir reglages

if (analogRead(7) == 1023){  

    if(tm.Minute == 00)
      setTime(tm.Hour-1,59,0,0,0,0);
    else
      setTime(tm.Hour,tm.Minute-1,0,0,0,0);
    RTC.set(now());
    delay(200);
}
if (digitalRead(12)== HIGH){
    if(tm.Minute == 59)
      setTime(tm.Hour+1,0,0,0,0,0);
    else
      setTime(tm.Hour,tm.Minute+1,0,0,0,0);
    RTC.set(now());
    delay(200);
}

// ******************************************* 2 points
/*
if(sec!=now.second()){
  sec=now.second();
  if(digitalRead(10)== HIGH)
    digitalWrite(10, LOW);
  else
    digitalWrite(10, HIGH);
}*/

//********************************************* radio

  if (mySwitch.available()) {
      value = mySwitch.getReceivedValue();
  
      if (value == 0) {
        Serial.print("Unknown encoding");
      } else {
        Serial.print("Received ");
        Serial.print( mySwitch.getReceivedValue(),DEC );
        Serial.print(" / ");
        Serial.print( mySwitch.getReceivedBitlength() );
        Serial.print("bit ");
        Serial.print("Protocol: ");
        Serial.println( mySwitch.getReceivedProtocol() );
  
      colors[-1+decalage] = (rgb_color){ 200, 200, 200};    // affichage des 4 led de directions
      colors[14+decalage] = (rgb_color){ 200, 200, 200};
      if(29+decalage>59)
        colors[29+decalage-60] = (rgb_color){ 200, 200, 200};
      else
        colors[29+decalage] = (rgb_color){ 200, 200, 200};
      if(44+decalage>59)
        colors[44+decalage-60] = (rgb_color){ 200, 200, 200};
      else
        colors[44+decalage] = (rgb_color){ 200, 200, 200};
      ledStrip.write(colors, LED_COUNT); 

      // a 1326849  b 1326850 c 1326852 d 1326856

        if( mySwitch.getReceivedValue() ==1326849 ){
          if(tm.Minute == 00)
            setTime(tm.Hour-1,59,tm.Second,0,0,0);
          else
            setTime(tm.Hour,tm.Minute-1,tm.Second,0,0,0);
          RTC.set(now());
          delay(200);
        }
        if( mySwitch.getReceivedValue() ==1326850 ){
          if(tm.Minute == 59)
            setTime(tm.Hour+1,0,tm.Second,0,0,0);
          else
            setTime(tm.Hour,tm.Minute+1,tm.Second,0,0,0);
          RTC.set(now());
          delay(200);
        }
        if( mySwitch.getReceivedValue() ==1326856 ){
          if(decalage > 1)
            decalage --;
          delay(200);
          for(int t = 0; t < LED_COUNT; t++)
            colors[t] = (rgb_color){ 0, 0, 0};
        }
        if( mySwitch.getReceivedValue() ==1326852 ){
          if(decalage < 32)
            decalage ++;
          delay(200);
          for(int t = 0; t < LED_COUNT; t++)
            colors[t] = (rgb_color){ 0, 0, 0};
        }

      }
      mySwitch.resetAvailable();
  }
}

//********************************************************** chiffre affichage
void chiffre(int val){
  switch(val){
    case 1:
    digitalWrite(8, 0);    digitalWrite(7, 0);
    break;
    case 2:
    digitalWrite(3, 0);    digitalWrite(5, 0);    digitalWrite(6, 0);    digitalWrite(8, 0);    digitalWrite(9, 0);
    break;
    case 3:
    digitalWrite(3, 0);    digitalWrite(6, 0);    digitalWrite(7, 0);    digitalWrite(8, 0);    digitalWrite(9, 0);    break;
    case 4:
    digitalWrite(3, 0);    digitalWrite(4, 0);    digitalWrite(7, 0);    digitalWrite(8, 0);
    break;
    case 5:
    digitalWrite(3, 0);    digitalWrite(4, 0);    digitalWrite(6, 0);    digitalWrite(7, 0);    digitalWrite(9, 0);
    break;
    case 6:
    digitalWrite(3, 0);    digitalWrite(4, 0);    digitalWrite(5, 0);    digitalWrite(6, 0);    digitalWrite(7, 0);    digitalWrite(9, 0);
    break;
    case 7:
    digitalWrite(7, 0);    digitalWrite(8, 0);    digitalWrite(9, 0);
    break;
    case 8:
    digitalWrite(3, 0);    digitalWrite(4, 0);    digitalWrite(5, 0);    digitalWrite(6, 0);    digitalWrite(7, 0);    digitalWrite(8, 0);    digitalWrite(9, 0);
    break;
    case 9:
    digitalWrite(3, 0);    digitalWrite(4, 0);    digitalWrite(6, 0);    digitalWrite(7, 0);    digitalWrite(8, 0);    digitalWrite(9, 0);
    break;
    case 0:
    digitalWrite(4, 0);    digitalWrite(5, 0);    digitalWrite(6, 0);    digitalWrite(7, 0);    digitalWrite(8, 0);    digitalWrite(9, 0);
    break;
  }
}

