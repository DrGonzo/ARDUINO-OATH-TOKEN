#include <LiquidCrystal.h>
#include "sha1.h"
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 RTC;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void printHash(uint8_t* hash) {
  int i;
  for (i=0; i<20; i++) Serial.println(hash[i]);
  Serial.println();
}

uint8_t hmacKey1[]={ 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21, 0xde, 0xad, 0xbe, 0xef, 0x6f, 0x21, 0xde, 0xad, 0xbe, 0xef };

long oldOtp = 0;

void setup() {
  Wire.begin();
  RTC.begin();
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("OTP: ");
}

int wait = 0;

void loop() {

  DateTime now = RTC.now();
  long GMT = (now.unixtime()-14400); // RTC_DS1307 'NOW' in GMT+4, converting to GMT
  
  uint8_t byteArray[8];   
  long time = GMT / 30;
              
  byteArray[0] = 0x00;
  byteArray[1] = 0x00;
  byteArray[2] = 0x00;
  byteArray[3] = 0x00;
  byteArray[4] = (int)((time >> 24) & 0xFF) ;
  byteArray[5] = (int)((time >> 16) & 0xFF) ;
  byteArray[6] = (int)((time >> 8) & 0XFF);
  byteArray[7] = (int)((time & 0XFF));
  
  uint8_t* hash;
  uint32_t a; 
  Sha1.initHmac(hmacKey1, sizeof(hmacKey1));
  Sha1.writebytes(byteArray, sizeof(byteArray));
  hash = Sha1.resultHmac();
  
  int  offset = hash[20 - 1] & 0xF; 
  long truncatedHash = 0;
  int j;
  for (j = 0; j < 4; ++j) {
    truncatedHash <<= 8;
    truncatedHash  |= hash[offset + j];
  }
    
  truncatedHash &= 0x7FFFFFFF;
  truncatedHash %= 1000000;
  
  
  if(truncatedHash != oldOtp){
    oldOtp = truncatedHash;
    wait = 48;
    Serial.println(truncatedHash);
    lcd.setCursor(5, 0);
    lcd.print(truncatedHash);
    lcd.setCursor(0, 1);
    lcd.print("\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83\x83");
  } else wait--;
  
  if(wait % 3 == 0){
    lcd.setCursor(wait/3, 1);
    lcd.print("\x80");
  }
  if((wait+1) % 3 == 0){
    lcd.setCursor(wait/3, 1);
    lcd.print("\x82");
  }
  if((wait+2) % 3 == 0){
    lcd.setCursor(wait/3, 1);
    lcd.print("\x81");
  }
  
  delay(610);
}
