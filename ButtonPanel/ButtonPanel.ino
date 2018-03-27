#include <SoftwareSerial.h>
#include "WIFI_Credentials.h"

const int UP = 8;
const int DOWN = 9;
const int LEFT = 10;
const int RIGHT = 11;
const int LED_PIN = 12;

String SERVER = "grid-dude.herokuapp.com";
int PORT = 80;

const int SOFT_RX=2, SOFT_TX=3;
SoftwareSerial softSerial(SOFT_RX, SOFT_TX);
#define wifiSerial Serial
#define debugSerial softSerial

byte presses[256];
int pressedCount = 0;

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

  pressedCount = 0;
  connectToWifi();

  turnLED(true);
  delay(100);
  turnLED(false);
  delay(100);
  turnLED(true);
  delay(100);
  turnLED(false);
  delay(100);
  turnLED(true);
  delay(1000);
  turnLED(false);
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
  wifiSerial.setTimeout(1000);
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

  wifiSerial.setTimeout(10000);
  eatUntil("WIFI CONNECTED\r\n", 3);
  eatUntil("WIFI GOT IP\r\n", 4);
  eatUntil("OK\r\n", 5);
  wifiSerial.setTimeout(1000);
  
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
  wifiSerial.println("AT+CIPCLOSE"); //Will give ERROR, seing as we are already closed
}

const String POST_str = "POST /update_grid HTTP/1.1\r\n";
const String HOST_str = "Host: "+SERVER+"\r\n";
const String CON_LEN_str = "Content-length: ";
void uploadPresses() {
  turnLED(true);
  String len(pressedCount);
  sendTCP(SERVER, PORT, POST_str.length()+HOST_str.length()+CON_LEN_str.length()+len.length()+2+2+pressedCount);
  wifiSerial.print(POST_str);
  wifiSerial.print(HOST_str);
  wifiSerial.print(CON_LEN_str);
  wifiSerial.print(len);
  wifiSerial.print("\r\n");
  wifiSerial.print("\r\n");
  wifiSerial.write(&presses[0], pressedCount);
  wifiSerial.setTimeout(10000);
  eatUntil("SEND OK\r\n", 8);
  eatUntil("success", 20);
  eatUntil("CLOSED", 15);
  wifiSerial.setTimeout(1000);
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
