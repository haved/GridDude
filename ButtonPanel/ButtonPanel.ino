#include <SoftwareSerial.h>
#include "WIFI_Credentials.h"

const int UP = 9;
const int DOWN = 10;
const int LEFT = 8;
const int RIGHT = 11;
const int LED_PIN = 12;

const int SOFT_RX=2, SOFT_TX=3;
SoftwareSerial softSerial(SOFT_RX, SOFT_TX);
#define wifiSerial Serial
#define debugSerial softSerial

void setup() {
	pinMode(UP, OUTPUT);
  pinMode(DOWN, OUTPUT);
  pinMode(LEFT, OUTPUT);
  pinMode(RIGHT, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  wifiSerial.begin(115200);
  while(!wifiSerial); //Wait

  debugSerial.begin(115200);
  while(!debugSerial); //Also wait

  connectToWifi();
}

int waitReadWifi() {
  turnLED(true);
  while(wifiSerial.available()==0);
  turnLED(false);
  return wifiSerial.read();
}

void eatUntil(String text) {
  if(!wifiSerial.find(&text[0]))
    errorLoop(10);
}

void eatOK() { eatUntil("OK\r\n"); }

void connectToWifi() {
  debugSerial.print("Connecting to wifi: ");
  wifiSerial.println("AT"); //Just to get an OK
  eatOK();
  wifiSerial.println("AT+RST"); //Reset
  eatUntil("ready\r\n");
  wifiSerial.println("AT+CWMODE=1"); //Client mode (I think)
  eatOK();
  wifiSerial.println("AT+CWQAP"); //Forget wifi network
  eatOK();
  wifiSerial.print("AT+CWJAP=\""); //Give network
  wifiSerial.print(WIFI_SSID);
  wifiSerial.print("\",\"");
  wifiSerial.print(WIFI_PASSWD);
  wifiSerial.println("\"");

  if(!wifiSerial.find("WIFI CONNECTED\r\n"))
      errorLoop(3);
  if(!wifiSerial.find("WIFI GOT IP\r\n"))
      errorLoop(4);
  eatOK();
  
  debugSerial.println("Connected to WiFi");
}

void sendHTTP(String server, int port) {
  wifiSerial.print("AT+CIPSTART=\"TCP\",\"");
  wifiSerial.print(server);
  wifiSerial.print("\",");
  wifiSerial.println(port);
  
}

void turnLED(bool state) {
  digitalWrite(LED_PIN, state?HIGH:LOW);
}

void errorLoop(int blinks) {
  for(int i = 0;;i++) {
    turnLED(true);
    delay(200);
    turnLED(false);
    delay(200);
    if(i%blinks==0)
      delay(1000);
  }
}

void loop() {
  turnLED(true);
  delay(1000);
  turnLED(false);
  delay(1000);
}

