/* 
  Ararat Mesrobian
  CPE 417
  12/5/2023
  GPS Module that will be connected to Blynk, and other platforms to notify users of their location
  Cheap solution to a navigation system as well as low power and customizable

  Code is taken from multiple included libraries


*/
#define BLYNK_AUTH_TOKEN "CHe3PNcqVbVke9SPuSOUhIhWwqP50t5i"
#define BLYNK_TEMPLATE_ID "TMPL2FxEvlIpd"
#define BLYNK_TEMPLATE_NAME "Ararat Mesrobian"
#define BLYNK_PRINT SerialUSB

#include <Adafruit_GPS.h>
#include <WiFiNINA.h>
#include <BlynkSimpleWiFiNINA.h>
#include <ArduinoMqttClient.h>
#include "ThingSpeak.h"

//GPS variables 
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false
uint32_t timer = millis();
//wifi variables
char ssid[] = "Verizon_4FC6XG"; // your network SSID (name)
char pass[] = "arms6-floe-dos"; // your network password
int status = WL_IDLE_STATUS; // the Wifi radio's status
WiFiClient client;
//Blynk Variables
char auth[] = BLYNK_AUTH_TOKEN;
uint32_t blynkTimer = millis();
float latitude;
char compassLat;
float longitude;
char compassLong;
float mph = 0;
//MQTT Variables
MqttClient mqttClient(client);
const char broker[] = "broker.emqx.io";
int port = 1883; //channel
const char topic1[] = "Latitude";
const char topic2[] = "Longitude";
const char topic3[] = "mph";
const long interval = 8000;
unsigned long previousMillis = 0;

//Thingspeak setup
unsigned long myChannelNumber = 2370424;
const char * myWriteAPIKey = "69DC41HZOHXHL7D0";
uint32_t tsTimer = millis();




void setup() {
  //Set up serial 
  Serial.begin(115200);
  Serial.print("Starting GPS System, one second\n");

  //set up GPS parameters
  setUpGPS();

  //set up Blynk
  Blynk.begin(auth, ssid, pass);

  //set up mqtt, this will be the publisher
  if(!mqttClient.connect(broker, port))
  {
    Serial.print("MQTT connection failed!\n");
  }
  mqttClient.connect(broker, port);

  //setup thingspeak connection
  ThingSpeak.begin(client);
}

void loop() {
  //GPS Printing
  //print the output from the GPS
  printGPS();

  //run blynk
  Blynk.run();
  if(millis() - blynkTimer > 1000) //update blynk every 1s, we dont want it to constantly send the same location since thats inefficient.
  {
      blynkTimer = millis();
      Blynk.virtualWrite(V0, latitude);
      Blynk.virtualWrite(V1, longitude);
      Blynk.virtualWrite(V2, mph);
      //Serial.print("Updated to blynk!\n");
  }

  //implement MQTT communication
  mqttClient.poll();
  printMQTT();
}

//Functions==================================================================================================================
void printMQTT(void)
{
  unsigned long currentMm = millis();
  if(currentMm - previousMillis >= interval)
  {
    previousMillis = currentMm;

    Serial.print(topic1);
    Serial.print(latitude);
    mqttClient.beginMessage(topic1);
    mqttClient.print(latitude);
    mqttClient.endMessage();

    Serial.print("\n");

    Serial.print(topic2);
    Serial.print(longitude);
    mqttClient.beginMessage(topic2);
    mqttClient.print(longitude);
    mqttClient.endMessage();

    Serial.print("\n");

    Serial.print(topic3);
    Serial.print(mph);
    mqttClient.beginMessage(topic3);
    mqttClient.print(mph);
    mqttClient.endMessage();

    Serial.print("\n");
  }
}

void setUpGPS(void)
{
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  GPSSerial.println(PMTK_Q_RELEASE);
}

void printGPS(void)
{
  char sentence = GPS.read(); //data goes here into sentence

  if(GPS.newNMEAreceived())
  {
    //Serial.print(GPS.lastNMEA());
    if(!GPS.parse(GPS.lastNMEA()))
    {
      return;
    }
  }

  if(millis() - timer > 2000)
  {
    //print the current time, returned by the satellites
    timer = millis();
    Serial.print("\nTime: ");
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.print("\n");
    //print coordincates when a fix is recieved
    if(GPS.fix)
    {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); //print coordinate(number)
      Serial.print(GPS.lat); //print compass direction(character)
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); //print coordinate(number)
      Serial.println(GPS.lon); //print compass direction(character)
      Serial.print("Speed (knots): "); 
      Serial.println(GPS.speed);
      Serial.print("Altitude: "); 
      Serial.println(GPS.altitude);

      //send to blynk
      latitude = GPS.latitude / 100;
      compassLat = GPS.lat;
      longitude = GPS.longitude / 100;
      compassLong = GPS.lon;
      mph = GPS.speed * 1.5; //convert from knots to mph
    }
  }
}
