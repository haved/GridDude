#include <SoftwareSerial.h>
#include "WIFI_Credentials.h"

const int LEFT = 8;
const int RIGHT = 9;
const int UP = 10;
const int DOWN = 11;
const int LED_PIN = 12;

String SERVER = "grid-dude.herokuapp.com";
int PORT = 80;

const int SOFT_RX=2, SOFT_TX=3;
SoftwareSerial softSerial(SOFT_RX, SOFT_TX);
#define wifiOut softSerial
#define wifiIn Serial
#define debugOut Serial

const int pressBufferSize = 512;
byte presses[pressBufferSize];
int pressedStart = 0;
int pressedEnd = 0;

void setup() {
	pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);
  pinMode(LED_PIN, OUTPUT);

  softSerial.begin(115200);
  while(!softSerial); //Wait

  Serial.begin(115200);
  while(!Serial); //Also wait

  wifiIn.setTimeout(1000);

  pressedStart = pressedEnd = 0;
  connectToWifi();

  // ..- to show that we are ready
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

void wifiPrint(String text) {
  wifiOut.print(text);
  debugOut.print("wifiPrint(");
  debugOut.print(text);
  debugOut.println(")");
}

void wifiPrintln(String text) {
  wifiOut.println(text);
  debugOut.print("wifiPrintln(");
  debugOut.print(text);
  debugOut.println(")");
}

int wifiRead() {
  int in = wifiIn.read();
  debugOut.write(in);
  return in;
}

void wifiWrite(byte* buf, int len) {
  wifiOut.write(buf, len);
  debugOut.write(buf, len);
  debugOut.println("-End of write");
}

int waitReadWifi() {
  turnLED(true);
  while(wifiIn.available()==0);
  turnLED(false);
  return wifiRead();
}

void eatUntil(String text, int errorCode) {
  if(!wifiIn.find(&text[0])) {
    debugOut.print("Timed out waiting for: ");
    debugOut.println(text);
    errorLoop(errorCode);
  }
}

void eatOK() { eatUntil("OK\r\n", 10); }

void connectToWifi() {
  debugOut.print("Connecting to wifi: ");
  wifiPrintln("AT"); //Just to get an OK
  eatOK();
  wifiPrintln("AT+RST"); //Reset
  eatUntil("ready\r\n", 11);
  wifiPrintln("AT+CWMODE=1"); //Client mode (I think)
  eatOK();
  wifiPrintln("AT+CWQAP"); //Forget wifi network
  eatOK();
  wifiPrint("AT+CWJAP=\""); //Give network
  wifiPrint(WIFI_SSID);
  wifiPrint("\",\"");
  wifiPrint(WIFI_PASSWD);
  wifiPrintln("\"");

  wifiIn.setTimeout(10000);
  eatUntil("WIFI CONNECTED\r\n", 3);
  eatUntil("WIFI GOT IP\r\n", 4);
  eatUntil("OK\r\n", 5);
  wifiIn.setTimeout(1000);
  
  debugOut.println("Connected to WiFi");
}

void sendTCP(String server, int port, int byteCount) {
  wifiPrint("AT+CIPSTART=\"TCP\",\"");
  wifiPrint(server);
  wifiPrint("\",");
  wifiPrintln(String(port));
  collectButtonsUntil("CONNECT\r\n", 6000, 13);
  collectButtonsUntil("OK\r\n", 6000, 12);
  wifiPrint("AT+CIPSEND=");
  wifiPrintln(String(byteCount));
  collectButtonsUntil("OK\r\n", 6000, 11);
}

const String POST_str = "POST /update_grid HTTP/1.1\r\n";
const String HOST_str = "Host: "+SERVER+"\r\n";
const String CON_LEN_str = "Content-length: ";
void uploadPresses() {
  debugOut.println("Starting upload");
  wifiIn.setTimeout(10);
  turnLED(true);

  int ps = pressedStart;
  int pe = pressedEnd;
  pressedStart = pressedEnd;

  int pressCount = pe-ps;
  if(pressCount < 0)
    pressCount += pressBufferSize;
  
  String pressCount_str(pressCount);
  sendTCP(SERVER, PORT, POST_str.length()+HOST_str.length()+CON_LEN_str.length()+pressCount_str.length()+2+2+pressCount);
  wifiPrint(POST_str);
  wifiPrint(HOST_str);
  wifiPrint(CON_LEN_str);
  wifiPrint(pressCount_str);
  wifiPrint("\r\n");
  wifiPrint("\r\n");

  if(ps + pressCount > pressBufferSize) {
    wifiWrite(&presses[ps], pressBufferSize-ps);
    wifiWrite(&presses[0], pe);
  } else {
    wifiWrite(&presses[ps], pressCount);
  }
  
  collectButtonsUntil("SEND OK\r\n", 10000, 8);
  collectButtonsUntil("success", 10000, 20);
  collectButtonsUntil("CLOSED", 10000, 15);
  //wifiPrintln("AT+CIPCLOSE"); //We are already closed
  turnLED(false);
  wifiIn.setTimeout(1000);
  debugOut.println("Upload finished");
}

const int timeBetweenChecks = 20;
void collectButtonsFor(int mil) {
  while(mil > 0) {
    static bool prevUp = false, prevDown = false, prevLeft = false, prevRight = false;
    bool up = digitalRead(UP), down = digitalRead(DOWN), left = digitalRead(LEFT), right = digitalRead(RIGHT);

    if(up && !prevUp) {
      presses[pressedEnd++] = 'U';
      pressedEnd%=pressBufferSize;
    }
    if(down && !prevDown) {
      presses[pressedEnd++] = 'D';
      pressedEnd%=pressBufferSize;
    }
    if(left && !prevLeft) {
      presses[pressedEnd++] = 'L';
      pressedEnd%=pressBufferSize;
    }
    if(right && !prevRight) {
      presses[pressedEnd++] = 'R';
      pressedEnd%=pressBufferSize;
    }
    
    prevUp = up; prevDown = down; prevLeft = left; prevRight = right;

    delay(timeBetweenChecks);
    mil-=timeBetweenChecks;
  }
}

const int findBufferSize = 128;
int findBufferIndex = 0;
char findBuffer[findBufferSize];
void collectButtonsUntil(String text, int timeout, int errorCode) {
  unsigned long keepTryingUntil = millis() + timeout;
  while(millis() < keepTryingUntil) {
     if(wifiIn.find(&text[0]))
        return;
     collectButtonsFor(1);
  }
  debugOut.print("Timed out waiting on: ");
  debugOut.println(text);
  errorLoop(errorCode);
}

void loop() {
  collectButtonsFor(1);
  
  static unsigned long noPressesLastTime=millis();
  if(pressedStart == pressedEnd) {
    noPressesLastTime = millis();
  }
  else {
    unsigned long diff = millis() - noPressesLastTime;
    if(diff > 10000 || (pressedEnd-pressedStart+pressBufferSize)%pressBufferSize > 10) {
      noPressesLastTime = millis();
      uploadPresses();
    }
  }
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
