//Libraries
#include "MeanFilterLib.h"
#include <ESP8266WiFi.h>
#include <Time.h>
#include <ArduinoJson.h>

//Pin I/O Labeling
#define CurrSensor 0  //IR distance sensor connected to Arduino analog pin 0

MeanFilter<float> meanFilter(10);
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
float Sensibilidad=0.030736;
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
unsigned int tiempoMax = 500;
unsigned int t1;
int relay = 0;
String devices[2];
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
  
  if(client.connect(host_azure, 80)){
        client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host_azure + "\r\n" +
                 "Content-Type: text/html\r\n\r\n");
    
    unsigned long timeout = millis();
    
    while (client.available() == 0) {
      if (millis() - timeout > 10000000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    int cont = 0;
     while (client.available()) {
      String line = client.readStringUntil('\r');
      cont++;
      if(cont==8){
        devices[0] = devices[1];
        devices[1] = line;
        Serial.print("Actual: ");
        Serial.println(devices[1]);
        Serial.print("Anterior: ");
        Serial.println(devices[0]);
      }
    } 
  }
  
}

void tiempo(){
  t1 = millis();
}

void setup() {
  pinMode(relay,OUTPUT);
  digitalWrite(relay,LOW);
  //Communications initialization
  Serial.begin(115200);
  WiFiInit();
}

void loop() {
  propertyValues[0] = get_corriente(200);
  delay(500);
  tiempo();
  if((devices[0]== devices[1]) && (millis()-t1 >= tiempoMax)){
    digitalWrite(relay,HIGH);
    tiempo();
    while(millis() -t1 <= tiempoMax){
      tiempo();
    }
  }
  digitalWrite(relay,LOW);
  if (millis() - lastConnectionTime > TPOST) {
    POST_Azure(propertyValues);
    lastConnectionTime = millis();
    Serial.println(propertyValues[0],3);  
  }
}

float get_corriente(int n_muestras)
{
  float voltajeSensor;
  float corriente=0;
  for(int i=0;i<n_muestras;i++)
  {
    voltajeSensor = analogRead(A0) * (3.3 / 1023.0);////lectura del sensor
    corriente=corriente+(voltajeSensor-1.170)/Sensibilidad; //Ecuación  para obtener la corriente
  }
  corriente=corriente/n_muestras;
  return(corriente+0.120);
}


