/*
This access system main door with ESP initial test
*/
//Annotation from TFT_lib author
// IMPORTANT: Adafruit_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h FOR SETUP.

// Modified for SPFD5408 Library by Joao Lopes
// and touch instead serial !!!!
// Too much modifications, so not: begins e ends
// Version 0.9.2 - Rotation for Mega and screen initial
///////////////

#include "Arduino.h"

#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>     // Touch library

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10 //SS pin for rfid
#define RST_PIN A1  //RST pin for rfid (not used)

void IDRead(byte *buffer, byte bufferSize);
String utf8rus(String source);

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
String ID = ""; // read ID from RFID
// Init array that will store new NUID
byte nuidPICC = 0;

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A5 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0x000F

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Calibrates value
// #define SENSIBILITY 50
// #define MINPRESSURE 10
// #define MAXPRESSURE 1000

//These are the pins for the shield!
// #define YP A1
// #define XM A2
// #define YM 7
// #define XP 6

// Calibrate values
// #define TS_MINX 125
// #define TS_MINY 85
// #define TS_MAXX 965
// #define TS_MAXY 905

// Init TouchScreen:
//TouchScreen ts = TouchScreen(XP, YP, XM, YM, SENSIBILITY);



void setup()
{
  tft.begin(0x9341); // SDFP5408
  tft.cp437(true);
  tft.setRotation(-1); // Need for the Mega, please changed for your choice or rotation initial

  pinMode(A5, OUTPUT);

  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  // Initial screen
  tft.fillScreen(WHITE);
  tft.setCursor (35, 85);
  tft.setTextSize (5);
  tft.setTextColor(BLUE);
  tft.println(utf8rus("Дверь-карта"));
  delay (10);
  tft.fillScreen(WHITE);

}

void loop()
{
  tft.setCursor (45, 200);
  tft.setTextSize (3);
  tft.setTextColor(RED);
  tft.println(utf8rus("Вставте карту"));

  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

   // Verify if the NUID has been readed
   if ( ! rfid.PICC_ReadCardSerial())
    return;

    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    //Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC)
   {
     nuidPICC = rfid.uid.uidByte[0];

     IDRead(rfid.uid.uidByte, rfid.uid.size);
     tft.fillScreen(WHITE);
     tft.setCursor (45, 100);
     tft.setTextSize (3);
     tft.setTextColor(GREEN);
     tft.println(ID);
   }

 // Halt PICC
 rfid.PICC_HaltA();

 // Stop encryption on PCD
 rfid.PCD_StopCrypto1();
}

//fnc for rus letters display
String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}

/**
* Helper routine to dump a byte array as dec values.
*/
void IDRead(byte *buffer, byte bufferSize)
{
 ID = "";
 for (byte i = 0; i < bufferSize; i++)
 {
   ID.concat(String(buffer[i], DEC));
 }
}
