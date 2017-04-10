#include "ESP8266WiFi.h"

//How many values you will be pushing to ThingWorx
#define propertyCount 1

const char* ssid     = "Logan";
const char* password = "";
char server[] = "iot.dis.eafit.edu.co";
// Use WiFiClient class to create TCP connections
WiFiClient client;

//ThingWorx App key which replaces login credentials)
char appKey[] = "354adb08-f33a-4214-a599-66eafa18042a";
// ThingWorx Thing name for which you want to set properties values
char thingName[] = "TomaAilet_thing";
//Interval of time at which you want the properties values to be sent to TWX server
int timeBetweenRefresh = 5000;
// ThingWorx service that will set values for the properties you need
// See the documentation for this tutorial for more information
char serviceName[] = "setCurrent";

//Initialize Properties Names and Values Arrays
char* propertyNames[] = {"Curr"};
double propertyValues[propertyCount];

// last time you connected to the server, in milliseconds
unsigned long lastConnectionTime = 0;
// state of the connection last time through the main loop
boolean lastConnected = false;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  conectarWifi();
  //POST(); // Post the value to the web database
  //GET();  // If all you need to do is push data, you don't need this
}

void loop() {
  // put your main code here, to run repeatedly:
  float current;
  propertyValues[0] = analogRead(0);
  if (millis() - lastConnectionTime > timeBetweenRefresh) {
    updateValues(propertyValues, client, server, appKey, thingName, serviceName, propertyNames);
  }
}

void updateValues(double values[] , WiFiClient &client, char server[], char appKey[], char thingName[], char serviceName[], char* sensorNames[])
{
  //build the String with the data that you will send
  //through REST calls to your TWX server
  char data[80];
  strcpy(data, "?appKey=");
  strcat(data, appKey);
  strcat(data, "&method=post&x-thingworx-session=true");
  // if you get a connection, report back via serial:
  if (client.connect(server, 8080)) {
    Serial.println("connected");
    // send the HTTP POST request:
    client.print("POST /Thingworx/Things/");
    client.print(thingName);
    Serial.println(thingName);
    client.print("/Services/");
    client.print(serviceName);
    client.print(data);
    client.print("<");
    for (int idx = 0; idx < propertyCount; idx++)
    {
      client.print("&");
      client.print(propertyNames[idx]);
      client.print("=");
      client.print(propertyValues[idx]);
    }
    client.print(">");
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Content-Type: text/html");
    client.println();
    int timeout = 0;
    while (!client.available() && timeout > 5000) {
      delay(1);
      timeout++;
    }
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    client.stop();
    lastConnectionTime = millis();

    // print the request out
    Serial.print("POST /Thingworx/Things/");
    Serial.print(thingName);
    Serial.print("/Services/");
    Serial.print(serviceName);
    Serial.print(data);
    Serial.print("<");
    for (int idx = 0; idx < propertyCount; idx++)
    {
      Serial.print("&");
      Serial.print(propertyNames[idx]);
      Serial.print("=");
      Serial.print(propertyValues[idx]);
    }
    Serial.print(">");
    Serial.println(" HTTP/1.1");
    Serial.print("Host: ");
    Serial.println(server);
    Serial.println("Content-Type: text/html");
    Serial.println();


  }
  else {
    // kf you didn't get a connection to the server:
    Serial.println("the connection could not be established");
    client.stop();
  }
}

void conectarWifi() {
  Serial.println();
  Serial.println(" Iniciando conexión ");
  WiFi.begin(ssid, password );
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado, dirección IP: ");
  Serial.println(WiFi.localIP());
}

void GET()
{
  Serial.print("connecting to ");
  Serial.println(server);

  const int httpPort = 80;
  if (!client.connect(server, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  // Make a HTTP request:
  client.println("GET /outlets HTTP/1.1");
  client.print("Host: ");
  client.println(server);
  client.println("Connection: close");
  client.println();
  delay(500);
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}

void POST()
{
  Serial.print("connecting to ");
  Serial.println(server);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(server, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.println("Requesting POST: ");
  client.print("POST /outlets/");
  client.print("?description=");
  //client.print(a);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server);
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.println("Cache-Control: no-cache");
  client.println("Connection: close");
  client.println();
  delay(500); // Can be changed

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");
}