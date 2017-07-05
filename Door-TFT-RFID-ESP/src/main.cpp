/*
This access system main door with ESP initial test
Send Card ID via MQTT
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
#include "avr/wdt.h"

#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>     // Touch library

#include <SPI.h>
#include <MFRC522.h>

#include <ELClient.h>
#include <ELClientCmd.h>
#include <ELClientMqtt.h>

#define SS_PIN 10 //SS pin for rfid
#define RST_PIN A1  //RST pin for rfid (not used)

void IDRead(byte *buffer, byte bufferSize);
String utf8rus(String source);

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
String ID = ""; // read ID from RFID
//const char * buf;
// Init array that will store new NUID
byte nuidPICC = 0;

static uint32_t last; //variable for asynchronous deleay
bool flag_screen = true;  //flag for screan change
/*
TFT Initialize block
*/
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
//End TFT Init
/*
esp-link Initialize block
*/
// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
ELClient esp(&Serial, &Serial);

// Initialize CMD client (for GetTime)
ELClientCmd cmd(&esp);

// Initialize the MQTT client
ELClientMqtt mqtt(&esp);

// Callback made from esp-link to notify of wifi status changes
// Here we just print something out for grins
void wifiCb(void* response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    // if(status == STATION_GOT_IP) {
    //   Serial.println("WIFI CONNECTED");
    // } else {
    //   Serial.print("WIFI NOT READY: ");
    //   Serial.println(status);
    // }
  }
}

bool connected;
const char* cardID_mqtt = "AS/admin/cardID";
const char* server_resp = "AS/door1/server_response";

// Callback when MQTT is connected
void mqttConnected(void* response) {
  Serial.println("MQTT connected!");
  //mqtt.subscribe(cardID_mqtt);
  //mqtt.subscribe(server_resp);

  connected = true;
}

// Callback when MQTT is disconnected
void mqttDisconnected(void* response) {
  Serial.println("MQTT disconnected");
  connected = false;
}

// Callback when an MQTT message arrives for one of our subscriptions
void mqttData(void* response) {
  ELClientResponse *res = (ELClientResponse *)response;

  Serial.print("Received: topic=");
  String topic = res->popString();
  Serial.println(topic);

  Serial.print("data=");
  String data = res->popString();
  Serial.println(data);

  if (topic == server_resp) {
    if (data == "yes") {
      tft.fillScreen(WHITE);
      tft.setCursor (45, 100);
      tft.setTextSize (3);
      tft.setTextColor(GREEN);
      tft.println(utf8rus("Відчинено"));
      nuidPICC = 0;
    }
    else if (data == "no") {
      tft.fillScreen(WHITE);
      tft.setCursor (45, 100);
      tft.setTextSize (3);
      tft.setTextColor(RED);
      tft.println(utf8rus("Відмова"));
      nuidPICC = 0;
    }

    last = millis();
    flag_screen = true;
    return;
  }
}

void mqttPublished(void* response) {
  //Serial.println("MQTT published");
}
//end esp-link Initialize block

//void(* resetFunc) (void) = 0;//объявляем функцию reset с адресом 0

//setup Arduino
void setup()
{
  //delay(15000);
  //esp MQTT setup
  Serial.begin(115200);
  Serial.println("EL-Client starting!");

  //
  // }
  // Sync-up with esp-link, this is required at the start of any sketch and initializes the
  // callbacks to the wifi status change callback. The callback gets called with the initial
  // status right after Sync() below completes.
  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  bool ok;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");



  // Set-up callbacks for events and initialize with es-link.
 mqtt.connectedCb.attach(mqttConnected);
 mqtt.disconnectedCb.attach(mqttDisconnected);
 mqtt.publishedCb.attach(mqttPublished);
 mqtt.dataCb.attach(mqttData);
 mqtt.setup();
 if (connected)
 {
   Serial.println("EL-MQTT ready");
 }
 //Serial.println("EL-MQTT ready");
 //esp.Process();
 //   delay(1000);
 //
 //   if (connected != true)
 //     {
 //       resetFunc(); //вызываем reset
 //     }

 // if (!connected)
 // {
 //   wdt_enable(WDTO_2S);
 //   while (!connected) ;
 //   wdt_disable();
 // }
// Serial.println("EL-MQTT ready");

 //tft setup
 //delay(2000);
 tft.begin(0x9341); // SDFP5408
 tft.cp437(true);
 tft.setRotation(-1); // Need for the Mega, please changed for your choice or rotation initial

 pinMode(A5, OUTPUT);

 //Serial.begin(9600);
 SPI.begin(); // Init SPI bus
 rfid.PCD_Init(); // Init MFRC522

 tft.fillScreen(WHITE);
 tft.setCursor (35, 85);
 tft.setTextSize (3);
 tft.setTextColor(BLUE);
 tft.println(utf8rus("Дверь-карта"));

 last = millis(); //start delay

}

//main program loop
//static int count;

void loop()
{
  esp.Process();

  if (!connected)
  {
    wdt_enable(WDTO_2S);
    while (!connected) ;
    wdt_disable();
  }


  if (flag_screen && (millis()-last) > 2000) {
    tft.fillScreen(WHITE);
    tft.setCursor (20, 100);
    tft.setTextSize (3);
    tft.setTextColor(RED);
    tft.println(utf8rus("Піднесіть карту"));
    nuidPICC = 0;
    flag_screen = false;
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
    //Serial.println(F("Your tag is not of type MIFARE Classic."));
    tft.fillScreen(WHITE);
    tft.setCursor (45, 100);
    tft.setTextSize (3);
    tft.setTextColor(RED);
    tft.println(utf8rus("Your tag is not of type MIFARE Classic."));
    last = millis();
    flag_screen = true;
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC)
   {
     nuidPICC = rfid.uid.uidByte[0];

     IDRead(rfid.uid.uidByte, rfid.uid.size); //byte ID to DEC
     //print ID on screan
     tft.fillScreen(WHITE);
     tft.setCursor (45, 100);
     tft.setTextSize (3);
     tft.setTextColor(GREEN);
     tft.println(utf8rus("Зчитано"));

     tft.setCursor (45, 200);
     tft.setTextSize (3);
     tft.setTextColor(GREEN);
     tft.println(ID);

     char buf[ID.length()+1];
     ID.toCharArray(buf, ID.length()+1);
     mqtt.publish(cardID_mqtt, buf);

     last = millis();
     flag_screen = true;
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
