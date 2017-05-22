//Libraries
#include "MeanFilterLib.h"
#include <ESP8266WiFi.h>
#include <Time.h>
#include <ArduinoJson.h>

//Pin I/O Labeling
#define CurrSensor 0  //IR distance sensor connected to Arduino analog pin 0

MeanFilter<float> meanFilter(43);
//Constants
const unsigned long TPOST = 1000;  //Delay between TWX POST updates, 1000 milliseconds
const unsigned int sensorCount = 1;  //Number of sensor vars sent to TWX, 1 var
const char* ssid = "JDONADO"; //WiFi SSID
const char* password = "12345678"; //WiFi Pass
const char* host = "molpe.eafit.edu.co";  //TWX Host
const char* host_azure = "neutron.eafit.edu.co";
//Variables
//->WiFi Shield Vars
WiFiClient client;
WiFiClient sclient;
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

void POST_Azure(float values[]){
  String url = "/toymoodle/local/integracion_ayre/ailet_test/request_all.php?";
  url += "amps";
  url += "=";
  url += (values[0]);
  url += "&";
  url += "ailetID";
  url += "=";
  url += "02";
  url += "&";
  url += "device";
  url += "=";
  url += "Null";
  url += "&";
  url += "level";
  url += "=";
  url += "LOW";
  url += "&";
  url += "user";
  url += "=";
  url += "Felipe";
  
  Serial.println("haciendo POST a Azure"); 
  if(client.connect(host_azure, 80)){
    Serial.println("conected!!!");
        client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host_azure + "\r\n" +
                 "Content-Type: text/html\r\n\r\n");
    Serial.println("Vamos a ver si esto si imprime.....................");
    
    unsigned long timeout = millis();
    
    while (client.available() == 0) {
      if (millis() - timeout > 10000000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
     while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.println(line);
    }
    
  }
  
}




/*void POST_Thingworx(int sensorCount, char* sensorNames[], float values[]) {
 
  String url = "/Thingworx/Things/";
  url += thingName;
  url += "/Services/";
  url += serviceName;
  url += "?appKey=";
  url += appKey;
  url += "&method=post";
  url += "&x-thingworxsession=true";
  url += "&";
  url += propertyNames[0];
  url += "=";
  url += values[0]-1.0;
  url += "&";
  url += "active";
  url += "=";
  url += "True";
  url += "&";
  url += "address";
  url += "=";
  url += "EAFIT";
  url += "&";
  url += "ailetID";
  url += "=";
  url += "02";
  url += "&";
  url += "device";
  url += "=";
  url += "Celular";
  url += "&";
  url += "level";
  url += "=";
  url += "LOW";
  url += "&";
  url += "user";
  url += "=";
  url += "jarbol";
  // if you get a connection, report back via serial:
  if (client.connect(host, 8080)) {
    // send the HTTP POST request:
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Content-Type: text/html\r\n\r\n");
    // print the request out
    Serial.println(String("POST ") + url + " HTTP/1.1\r\n" +
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
}*/



void setup() {
 
  //Communications initialization
  Serial.begin(115200);
  WiFiInit();
}

void loop() {
  propertyValues[0] = meanFilter.AddValue(analogRead(0));
  //propertyValues[1] = 200;
  if (millis() - lastConnectionTime > TPOST) {
    POST_Azure(propertyValues);
    //delay(1000);
    //POST_Thingworx(sensorCount, propertyNames, propertyValues);

    lastConnectionTime = millis();

  }
}


