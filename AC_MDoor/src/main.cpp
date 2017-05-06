/*
this is door part
Read from RFID, send ID to server
Controlling the Bell and 2 light diods.

RFID
 * Typical pin layout used:
* ----------------------------------------------------------
*             MFRC522      Arduino
*             Reader/PCD   Nano v3
* Signal      Pin          Pin
* -----------------------------------------------------------
* RST/Reset   RST          D9
* SPI SS      SDA(SS)      D10
* SPI MOSI    MOSI         D11
* SPI MISO    MISO         D12
* SPI SCK     SCK          D13
*
* Create in Atom - PlatformIO
*/
#include "Arduino.h"

#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10 //SS pin for rfid
#define RST_PIN 9 //RST pin for rfid (not used)

#define led_green1 8
#define led_red1 7
#define buzzer 6
#define led_green2 5
#define led_red2 4

void printDec(byte *buffer, byte bufferSize); //declare helper function

SoftwareSerial mySerial(2, 3); // RX, TX

String msg = ""; // read from serial
String ID = ""; // read ID from RFID

int myTimeout = 50;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

// Init array that will store new NUID
byte nuidPICC = 0;

void setup() {
 mySerial.begin(9600);
 Serial.begin(9600);
 SPI.begin(); // Init SPI bus
 rfid.PCD_Init(); // Init MFRC522

 pinMode(led_green1,OUTPUT);
 pinMode(led_red1,OUTPUT);
 pinMode(led_green2,OUTPUT);
 pinMode(led_red2,OUTPUT);
 pinMode(buzzer,OUTPUT);

 Serial.setTimeout(myTimeout);
 mySerial.setTimeout(myTimeout);

}

void loop() {

   if (mySerial.available()) //do only if there is data on mySerial
 {
   msg = mySerial.readString();
   if (msg == "SD1#yes")
   {
     digitalWrite(led_green1, HIGH);
     digitalWrite(led_green2, HIGH);
     digitalWrite(buzzer, HIGH);
     delay (1000);
     digitalWrite(led_green1, LOW);
     digitalWrite(led_green2, LOW);
     digitalWrite(buzzer, LOW);
     nuidPICC = 0;
   }
   if (msg == "SD1#no")
   {
     digitalWrite(led_red1, HIGH);
     digitalWrite(led_red2, HIGH);
     delay (1000);
     digitalWrite(led_red1, LOW);
     digitalWrite(led_red2, LOW);
     nuidPICC = 0;
   }

  }

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
   mySerial.println(F("Your tag is not of type MIFARE Classic."));
   return;
 }

 if (rfid.uid.uidByte[0] != nuidPICC) //||
   {
     nuidPICC = rfid.uid.uidByte[0];

     printDec(rfid.uid.uidByte, rfid.uid.size);
     mySerial.print("D1#"+ID+"\n");
   }



 // Halt PICC
 rfid.PICC_HaltA();

 // Stop encryption on PCD
 rfid.PCD_StopCrypto1();



}

/**
* Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize)
{
 ID = "";
 for (byte i = 0; i < bufferSize; i++)
 {
   ID.concat(String(buffer[i], DEC));
 }
}
