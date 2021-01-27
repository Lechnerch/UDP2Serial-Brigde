// UDP2Serial-Brigde tested on a Wemos D1 mini 21.01.2021 by Lechnerch
// desigined for transmitting UDP-commands to a AGopenGPS Autosteer - Steering Wheel by Keya Electron http://www.dcmotorkeya.com/automatic-steering-motor-for-agricultural-machinery-auto-driving-system.html
// For converting the signal a RS232MAX-converter is needed too.
// It schould also work with any other kind of commands
// NO GUARANTEE, NO WARRANTY!!!! Improvements are welcome!
// Python Release 2.7.17 https://www.python.org/downloads/release/python-2717/

#include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include <ArduinoOTA.h>

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


void setup() {
  mySerial.begin(115200);
  Serial.begin(115200);
  WiFi.hostname("WemosD1mini_SteeringWheel");
  ArduinoOTA.setHostname("WemosD1mini_SteeringWheel_OTA");
  ArduinoOTA.setPassword("123");
  ArduinoOTA.setPort(65280);
  wificonnect();
  Udp.begin(UDPLocalPort);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
    Udp.beginPacket(IPRecipient, UDPRemotePort);
    Udp.println("Start updating: " + type );
    Udp.endPacket();

  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    Udp.beginPacket(IPRecipient, UDPRemotePort);
    Udp.println("End");
    Udp.endPacket();
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    Udp.beginPacket(IPRecipient, UDPRemotePort);
    Udp.println((progress / (total / 100)));
    Udp.endPacket();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
      Udp.beginPacket(IPRecipient, UDPRemotePort);
      Udp.println("Auth failed");
      Udp.endPacket();
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
      Udp.beginPacket(IPRecipient, UDPRemotePort);
      Udp.println("Begin failed");
      Udp.endPacket();
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
      Udp.beginPacket(IPRecipient, UDPRemotePort);
      Udp.println("Connect Failed");
      Udp.endPacket();
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
      Udp.beginPacket(IPRecipient, UDPRemotePort);
      Udp.println("Receive Failed");
      Udp.endPacket();
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
      Udp.beginPacket(IPRecipient, UDPRemotePort);
      Udp.println("End failed");
      Udp.endPacket();
    }
  });
  
  ArduinoOTA.begin();
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
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle(); // OTA handle, DONT REMOVE
  }
  
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

  if (mySerial.available() > 0) {
    mySerial.readBytes(UDPSendPacketBuffer, sizeof(UDPSendPacketBuffer));
    mySerial.flush();
    Udp.beginPacket(IPRecipient, UDPRemotePort);
    for (int i = 0; i < sizeof(UDPSendPacketBuffer); i++) {
      Udp.write(UDPSendPacketBuffer[i]);
      UDPSendPacketBuffer[i] = 0;
    }
    Udp.endPacket();
  }
}
