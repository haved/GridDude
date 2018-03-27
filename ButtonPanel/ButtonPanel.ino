#include <SoftwareSerial.h>
#include "WIFI_Credentials.h"

const int UP = 9;
const int DOWN = 10;
const int LEFT = 8;
const int RIGHT = 11;
const int LED_PIN = 12;

String SERVER = "grid-dude.herokuapp.com";
int PORT = 80;

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

void eatUntil(String text, int errorCode) {
  if(!wifiSerial.find(&text[0]))
    errorLoop(errorCode);
}

void eatOK() { eatUntil("OK\r\n", 10); }

void connectToWifi() {
  debugSerial.print("Connecting to wifi: ");
  Serial.setTimeout(1000);
  wifiSerial.println("AT"); //Just to get an OK
  eatOK();
  wifiSerial.println("AT+RST"); //Reset
  eatUntil("ready\r\n", 11);
  wifiSerial.println("AT+CWMODE=1"); //Client mode (I think)
  eatOK();
  wifiSerial.println("AT+CWQAP"); //Forget wifi network
  eatOK();
  wifiSerial.print("AT+CWJAP=\""); //Give network
  wifiSerial.print(WIFI_SSID);
  wifiSerial.print("\",\"");
  wifiSerial.print(WIFI_PASSWD);
  wifiSerial.println("\"");

  Serial.setTimeout(10000);
  eatUntil("WIFI CONNECTED\r\n", 3);
  eatUntil("WIFI GOT IP\r\n", 4);
  eatUntil("OK\r\n", 5);
  Serial.setTimeout(1000);
  
  debugSerial.println("Connected to WiFi");
}

void sendTCP(String server, int port, int byteCount) {
  wifiSerial.print("AT+CIPSTART=\"TCP\",\"");
  wifiSerial.print(server);
  wifiSerial.print("\",");
  wifiSerial.println(port);
  eatUntil("CONNECT\r\n", 13);
  eatOK();
  wifiSerial.print("AT+CIPSEND=");
  wifiSerial.println(byteCount);
  eatOK();
}

void endTCP() {
  eatUntil("SEND OK\r\n", 8);
  wifiSerial.println("AT+CIPCLOSE");
  eatOK();
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

byte presses[256];
int pressedCount = 0;

String POST_string = "POST /update_grid HTTP/1.1\r\n";
void uploadPresses() {
  turnLED(true);
  sendTCP(SERVER, PORT, POST_string.length() + pressedCount + 2);
  wifiSerial.print(POST_string);
  wifiSerial.write(&presses[0], pressedCount);
  wifiSerial.print("\r\n");
  endTCP();
  pressedCount = 0;
  turnLED(false);
}

void loop() {
  delay(10000);
  presses[0]='U';
  presses[1]='L';
  pressedCount = 2;
  uploadPresses();
}

