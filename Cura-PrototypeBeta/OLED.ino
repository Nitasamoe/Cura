

void rewriteDisplay(int newScore) {
  float score = newScore / 100.0;
  int breiteScore = 200 * score; 
  EPD.Dis_Clear_full();
  EPD.EPD_Init();
  EPD.EPD_WriteDispRamMono(200, breiteScore, 0x00);
  //EPD.Dis_Drawing(0,0,(unsigned char *)tobias,breiteScore,200);
  Serial.print("saved score : ");
  Serial.println(newScore);
  EPD.EPD_Update();
}


void saveScoreInEPROM(int shouldHour, int shouldMinute) {
    //Serial.print("saveScoreIn gestartet ");
    //rtcReadTime();
    //setRTCTime();

    int timestampCurrent = stunde * 60 + minute;    // jetzige Uhrzeit in Minuten
    int timestampShould = shouldHour * 60 + shouldMinute; // Uhrzeit wann genommen werden soll
    int minuteDifference = timestampCurrent - timestampShould; // differenz der zweien oberen
    
    //Serial.print("Minuten Differenz : ");
    //Serial.println(minuteDifference);

    float score = 1;
    if (minuteDifference <= 5) {
      score = 1.1;
    } else if (minuteDifference > 1 && minuteDifference <= 15) {
      score = 1.05;
    } else if ( minuteDifference > 15 ) {
      score = 0.9;
    }

    int average = EEPROM.read(scoreAddr);
    int endScore = score * average;
    if (endScore > 100) {
      endScore = 100;
    }
    
    EEPROM.write(scoreAddr, endScore);

    
  rewriteDisplay(endScore);

  rtcReadTime();
  printRTCDateTime(); 
  //Serial.print("saveScoreIn beendet ");
}
