// UDP2Serial-Brigde tested on a Wemos D1 mini 21.01.2021 by Lechnerch
// desigined for transmitting UDP-commands to a AGopenGPS Autosteer - Steering Wheel by Keya Electron http://www.dcmotorkeya.com/automatic-steering-motor-for-agricultural-machinery-auto-driving-system.html
// For converting the signal a RS232MAX-converter is needed too.
// It schould also work with any other kind of commands
// NO GUARANTEE, NO WARRANTY!!!! Improvements are welcome!
// AsyncElegantOTA Copyright (c) 2019 Ayush Sharma  https://github.com/ayushsharma82/AsyncElegantOTA

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h> // AsyncElegantOTA Copyright (c) 2019 Ayush Sharma  https://github.com/ayushsharma82/AsyncElegantOTA

#ifndef STASSID
#define STASSID "SSID"        // set SSID
#define STAPSK  "PASSWORD"    // set PASSWORD
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

unsigned int UDPLocalPort = 8877;       // local port the Wemos D1 mini listens on
unsigned int UDPRemotePort = 8866;      // port UDP-packets are sent to

IPAddress IPUdpToSerial(192, 168, 1, 72);   // set the own ip-adress
IPAddress IPgateway(192, 168, 1, 1);        // set the ip-adress of the gateway
IPAddress IPsubnet(255, 255, 255, 0);       // set the ip-adress of the subnet
IPAddress IPRecipient(192, 168, 1, 255);    // set the ip-adress where packets are sent too

byte UDPReceivePacketBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet,
byte UDPSendPacketBuffer[13];  // set the length of byte you expect
int i = 1;

WiFiUDP Udp;
SoftwareSerial mySerial(2, 0); // RX, TX  // On Wemos D1 mini RX(2) means Pin D4, TX(0) means Pin D3 // Have a look at https://arduino-projekte.info/wp-content/uploads/2017/03/wemos_d1_mini_pinout.png
AsyncWebServer server(80);

void setup() {
  mySerial.begin(115200);
  Serial.begin(115200);
  WiFi.hostname("WemosD1mini_SteeringWheel");
  wificonnect();
  Udp.begin(UDPLocalPort);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/html", "Got to <a href='../update'>192.168.1.72/update</a>");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

}

void wificonnect() {
  WiFi.mode(WIFI_STA);
  WiFi.config(IPUdpToSerial, IPgateway, IPsubnet);
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(2500);
  }
  Serial.println("");
  delay(2000);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("UDP server on port %d\n", UDPLocalPort);
  }
}


void loop() {
  AsyncElegantOTA.loop();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting");
    wificonnect();
    delay(1000);
  }

// Get data from wifi-UDP and send it to mySerial

  int UDPReceivePacketSize = Udp.parsePacket();
  if (UDPReceivePacketSize) {
    int n = Udp.read(UDPReceivePacketBuffer, UDP_TX_PACKET_MAX_SIZE);
    mySerial.write(UDPReceivePacketBuffer, 8);
    UDPReceivePacketBuffer[n] = 0;
    mySerial.flush();

  }

// Get data from mySerial and send it to wifi-UDP
/*
  if (mySerial.available() > 0) {
    mySerial.readBytes(UDPSendPacketBuffer, sizeof(UDPSendPacketBuffer));
    mySerial.flush();
    Udp.beginPacket(IPRecipient, UDPRemotePort);
    for (int i = 0; i < sizeof(UDPSendPacketBuffer); i++) {
      Udp.write(UDPSendPacketBuffer[i]);
      UDPSendPacketBuffer[i] = 0;
    }
    Udp.endPacket();
  }*/
}
