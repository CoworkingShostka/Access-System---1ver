/*
 This is server part
 Read from SoftwareSerial, wich connect to local object and Write to Serial
 Read from Serial and print to SoftwareSerial

 Create in Atom - PlatformIO
 */

#include <Arduino.h>

#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10 //SS for RFID
#define RST_PIN 9 // RST for RFID (not connect)

void printDec(byte *buffer, byte bufferSize);

SoftwareSerial mySerial(2, 3); // RX, TX
String ID = "";
String msg="";
int myTimeout = 50;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the RFID class

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.setTimeout(myTimeout);
  mySerial.setTimeout(myTimeout);

  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}

void loop() {

  if (mySerial.available()) {
    msg = mySerial.readString();
    Serial.print(msg);
  }

  if (Serial.available()) {
    msg = Serial.readString();
    mySerial.print(msg);
  }

  msg = "";

  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.print("G#"+ID+"\n");

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
