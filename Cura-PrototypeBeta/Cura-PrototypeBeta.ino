#include <Wire.h>
#define RTC_I2C_ADDRESS 0x68 // I2C Adresse des RTC  DS3231
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
// #include <ADXL345.h>

// E-Ink Includes -----------------------------------
#include "EPD_drive.h"
#include "EPD_drive_gpio.h"
#include "Display_Lib.h"

// - -----------------Uhrzeiten --------------------------------
int termineStund [4] = {18,21,21,21};
int termineMin [4] = {19,22,18,25};
int termineSek [4] = {0,0,0,0};


int lastStamp = 0;
// ---------------------------------------------------------------

// ----------------- Variablen für Uhr --------------------------------
int jahr,monat,tag,stunde,minute,sekunde, wochentag;
int daysInMonth[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
String daysInWeek[7] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
String monthInYear[12] = {"Januar","Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};
String outputFormat = "%s, %02d.%s %04d %02d:%02d:%02d Uhr";
// ---------------------------------------------------------------

// Button 
const int buttonPin = 3;  // Input Pin für Knopf
const int buttonOut1 = 2;  //Output Pin Motor vbration

// --------------------- EEPROM Variables ---------------------
const int scoreAddr = 0;
const int startScore = 50;
// -------------------------------------------------------------

const int vibrationDuration = 20;
int aktuellerTermin = 0;
int vibrationAusgeloest = false;
int timeVibrationStart = 0;

// E Ink Was auch immer
WaveShare_EPD EPD = WaveShare_EPD();

//Setup Methode
void setup(){
  Wire.begin(); //Kommunikation über die Wire.h bibliothek beginnen.
  
  Serial.begin(57600);  //Übertragungsgeschwindigkeit 9600 Baud

  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.begin();
  
// Pin Setup
  pinMode(buttonPin, INPUT);  // Pin Input für Knopf
  pinMode(buttonOut1, OUTPUT);  // OutPut Pin Motor Vibratio
  
// Pin Setup E Ink
   pinMode(CS,OUTPUT);
   pinMode(DC,OUTPUT);
   pinMode(RST,OUTPUT);
   pinMode(BUSY,INPUT);

  //resetFunc();

  EPD.Dis_Clear_full();
    
  rtcReadTime();
  printRTCDateTime(); 

  Serial.print("Ausgelesener Score = ");
  Serial.println(EEPROM.read(scoreAddr));
  
  rewriteDisplay(EEPROM.read(scoreAddr));  //takes the score read from EPROM and puts it to Display

 }

void loop(){

  //digitalWrite(buttonOut1, HIGH);
  
   rtcReadTime();
   //setRTCTime();

         if(digitalRead(buttonPin) == HIGH) {Serial.println("hallo");}


  int terminEinsNaehe = abs((termineStund[0] * 3600 + termineMin[0] * 60 + termineSek[0]) - ( stunde * 3600 + minute * 60 + sekunde ));
  int terminZweiNaehe = abs((termineStund[1] * 3600 + termineMin[1] * 60 + termineSek[1]) - ( stunde * 3600 + minute * 60 + sekunde ));
  int terminDreiNaehe = abs((termineStund[2] * 3600 + termineMin[2] * 60 + termineSek[2]) - ( stunde * 3600 + minute * 60 + sekunde ));
  int terminVierNaehe = abs((termineStund[3] * 3600 + termineMin[3] * 60 + termineSek[3]) - ( stunde * 3600 + minute * 60 + sekunde ));

  int differenz = ( stunde * 3600 + minute * 60 + sekunde ) - ( termineStund[aktuellerTermin] * 3600 + termineMin[aktuellerTermin] * 60 + termineSek[aktuellerTermin] );

  for(int x = 0; x < 4; x++) {
    int output = differenz;
    if(terminEinsNaehe < differenz) { aktuellerTermin = 0; };
    if(terminZweiNaehe < differenz) { aktuellerTermin = 1; };
    if(terminDreiNaehe < differenz) { aktuellerTermin = 2; };
    if(terminVierNaehe < differenz) { aktuellerTermin = 3; };
  }

  

   if(lastStamp != (stunde * 3600 + minute *60 + sekunde) /* && vibrationAusgeloest == false */ ) {
      lastStamp = stunde * 3600 + minute *60 + sekunde;
      printRTCDateTime();
      Serial.print("Differenz : ");
      Serial.println(differenz);

      Serial.print("vibrationAusgeloest : ");
      Serial.println(vibrationAusgeloest);

      Serial.print("aktuellerTermin : ");
      Serial.println(aktuellerTermin);

    }


    
    
    for(int x = 0; x < 4; x++) {
        if(termineStund[x] == stunde && termineMin[x] == minute && termineSek[x] == sekunde && vibrationAusgeloest == false){
            aktuellerTermin = x;
            Serial.print("aktuellerTermin : ");
            Serial.println(aktuellerTermin);

            Serial.print("h : ");
            Serial.println(stunde);
                        
            Serial.print("m : ");
            Serial.println(minute);
            
            Serial.print("s : ");
            Serial.println(sekunde);
                        
            vibrationAusgeloest = true;
        }
    }

    if( vibrationAusgeloest == true && differenz < 7 ) {
       digitalWrite(buttonOut1, HIGH);
       delay(100);
       digitalWrite(buttonOut1, LOW);
       delay(100);
    }
    if( vibrationAusgeloest == true && differenz > 300 ) {
      int score = EEPROM.read(scoreAddr)*0.8;
      
       digitalWrite(buttonOut1, LOW);
       vibrationAusgeloest = false;
       EEPROM.write(scoreAddr, score);
       rewriteDisplay(score);
    }
    
    if(digitalRead(buttonPin) == HIGH && vibrationAusgeloest == true) {

         digitalWrite(buttonOut1, LOW);
         vibrationAusgeloest = false;

         Serial.print("Stund : ");
         Serial.println(stunde);

         Serial.print("Minute : ");
         Serial.println(minute);

         Serial.print("Sekunde : ");
         Serial.println(sekunde);

    
         Serial.print("Differenz Sekunden : ");
         Serial.println(differenz);

        int score = EEPROM.read(scoreAddr);
         
        if(differenz == 122) {
          EEPROM.write(scoreAddr, score*1.3);
        } else if(differenz < 600) {
          EEPROM.write(scoreAddr, score*1.1);
        }
        if( ( EEPROM.read(scoreAddr) > 100 ) ) 
            { EEPROM.write(scoreAddr, 100); }
        rewriteDisplay(EEPROM.read(scoreAddr));
    }


}
