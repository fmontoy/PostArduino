//Libraries
#include "MeanFilterLib.h"
#include <ESP8266WiFi.h>
//#include <ArduinoJson.h>

//Pin I/O Labeling
#define CurrSensor 0  //IR distance sensor connected to Arduino analog pin 0
//#define flow 5  //Flow sensor connected to Arduino digital pin 5
//#define Electrovalve1 1  //Electrovalve (+) connected to Arduino pwm pin 1
//#define Electrovalve2 2  //Electrovalve (-) connected to Arduino pwm pin 1
//#define Pump1 3  //Pump (+) connected to Arduino pwm pin 3
//#define Pump2 4  //Pump (-) connected to Arduino pwm pin 3
MeanFilter<float> meanFilter(10);
//Constants
const unsigned long TPOST = 1000;  //Delay between TWX POST updates, 1000 milliseconds
const unsigned int sensorCount = 1;  //Number of sensor vars sent to TWX, 1 var
const char* ssid = "ailet"; //WiFi SSID
const char* password = "1234567890"; //WiFi Pass
const char* host = "molpe.eafit.edu.co";  //TWX Host

//Variables
//->WiFi Shield Vars
WiFiClient client;
//->TWX Vars
char appKey[] = "98a4eae5-d9cb-484f-aa34-857bfc7e934c"; //API Key from TWX
char thingName[] = "ailet_thing";  //Name of the thing in TWX
char serviceName[] = "setCurrent";  //Name of the service in TWX
char* propertyNames[] = {"current_value"};  //Vector Var names from TWX service inputs
float propertyValues[sensorCount]; //Vector for Var values
//char* writeNames[] = {"Write1", "Write2"};
//unsigned int Write1 = 0;  //Write1 value from twx (Electrovalve)
//unsigned int Write2 = 0;  //Write2 value from twx (Electrovalve)
//->Timing Vars
unsigned long lastConnectionTime = 0; //Last time you connected to the server, in milliseconds

//Subroutines & Functions
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void WiFiInit() {
  // give the module time to boot up:
  delay(1000);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid, password);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWifiStatus();
}

void POST(int sensorCount, char* sensorNames[], float values[]) {
  String url = "/Thingworx/Things/";
  url += thingName;
  url += "/Services/";
  url += serviceName;
  url += "?appKey=";
  url += appKey;
  url += "&method=post";
  url += "&x-thingworxsession=true";
  url += "<";
  for (int idx = 0; idx < sensorCount; idx++)
  {
    url += "&";
    url += propertyNames[idx];
    url += "=";
    url += values[idx];
  }
  url += ">";
  // if you get a connection, report back via serial:
  if (client.connect(host, 8080)) {
    Serial.println("connected");
    // send the HTTP POST request:
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Content-Type: text/html\r\n\r\n");

    // print the request out
    Serial.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Content-Type: text/html\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    // Read all the lines of the reply from server and print them to Serial
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }
  else {
    // kf you didn't get a connection to the server:
    Serial.println("the connection could not be established");
    client.stop();
  }
}

//void GET(char* writeNames[]) {
//  String urlG = "/Thingworx/Things/";
//  urlG += thingName;
//  urlG += "/Properties";
//  urlG += "?appKey=";
//  urlG += appKey;
//  urlG += "&x-thingworxsession=true";
//
//  // if you get a connection, report back via serial:
//  if (client.connect(host, 80)) {
//    Serial.println("connected");
//    // send the HTTP POST request:
//    client.print(String("GET ") + urlG + " HTTP/1.1\r\n" +
//                 "Host: " + host + "\r\n" +
//                 "Accept: application/json\r\n" +
//                 "Cache-Control: no-cache\r\n\r\n");
//
//    // print the request out
//    Serial.print(String("GET ") + urlG + " HTTP/1.1\r\n" +
//                 "Host: " + host + "\r\n" +
//                 "Accept: application/json\r\n" +
//                 "Cache-Control: no-cache\r\n\r\n");
//    unsigned long timeout2 = millis();
//    while (client.available() == 0) {
//      if (millis() - timeout2 > 5000) {
//        Serial.println(">>> Client Timeout !");
//        client.stop();
//        return;
//      }
//    }
//
//    // Read all the lines of the reply from server and print them to Serial
//    String json = "";
//    boolean httpBody = false;
//    while (client.available()) {
//      String line = client.readStringUntil('\r');
//      if (!httpBody && line.charAt(1) == '{') {
//        httpBody = true;
//      }
//      if (httpBody) {
//        json += line;
//        httpBody = false;
//      }
//    }
//    Serial.println(json);
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject& root = jsonBuffer.parseObject(json);
//
//    if (root.success()) {
//      Serial.println("JSON parsing Succeed!");
//    }
//    //root["rows"][0]["Write1"].printTo(Serial);
//    //Serial.println(nestedArray[0]);
//    Write1 = root["rows"][0][writeNames[0]];
//    Write2 = root["rows"][0][writeNames[1]];
//    Serial.print("Write1: ");
//    Serial.print(Write1);
//    Serial.print(" Write2: ");
//    Serial.println(Write2);
//  }
//  else {
//    // kf you didn't get a connection to the server:
//    Serial.println("the connection could not be established");
//    client.stop();
//  }
//}


void setup() {
  //Pin type definition
//  pinMode(Pump1, OUTPUT);
//  pinMode(Pump2, OUTPUT);
//  pinMode(Electrovalve1, OUTPUT);
//  pinMode(Electrovalve2, OUTPUT);
//  pinMode(LED_BUILTIN, OUTPUT);
//  pinMode(flow, INPUT);

  //Physical output cleaning
//  digitalWrite(Pump1, HIGH);
//  digitalWrite(Pump2, HIGH);
//  digitalWrite(Electrovalve1, HIGH);
//  digitalWrite(Electrovalve2, HIGH);
//  digitalWrite(LED_BUILTIN, HIGH);

  //Communications initialization
  Serial.begin(115200);
  WiFiInit();
}

void loop() {
  propertyValues[0] = meanFilter.AddValue(analogRead(0));
  //propertyValues[1] = 200;
  if (millis() - lastConnectionTime > TPOST) {
    POST(sensorCount, propertyNames, propertyValues);
    //GET(writeNames);
    lastConnectionTime = millis();
//    if (Write1 == 1) {
//      digitalWrite(Electrovalve1, LOW);
//      digitalWrite(Electrovalve2, LOW);
//      digitalWrite(LED_BUILTIN, LOW);
//    }
//    else {
//      digitalWrite(Electrovalve1, HIGH);
//      digitalWrite(Electrovalve2, HIGH);
//      digitalWrite(LED_BUILTIN, HIGH);
//    }
//    if (Write2 == 1) {
//      digitalWrite(Pump1, LOW);
//      digitalWrite(Pump2, LOW);
//    }
//    else {
//      digitalWrite(Pump1, HIGH);
//      digitalWrite(Pump2, HIGH);
//    }
  }
}


