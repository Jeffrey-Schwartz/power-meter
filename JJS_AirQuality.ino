//For temp/humidity sensor
#include <Wire.h>
#include "Adafruit_HTU21DF.h"

// most launchpads have a red LED
//see pins_energia.h for more LED definitions
//#define LED RED_LED
#define LED GREEN_LED

/*For WiFi*/
#ifndef __CC3200R1M1RGC__
// Do not include SPI for CC3200 LaunchPad
#include <SPI.h>
#endif
#include <WiFi.h>
// your network name also called SSID
//char ssid[] = "virus";
//char ssid[] = "NETGEAR40";
char ssid[] = "WeatherBot";
//char ssid[] = "UCLA_WEB";
// your network password
//char password[] = "Jeffreyissogreat";
//char password[] = "dynamicbanana708";
char password[] = "weather1";
//char password[] = "";
/*End Wifi*/

WiFiClient GET_client;
WiFiClient POST_client;
const char hostname[] = "weatherbot46.azurewebsites.net";
const String url = "/api/telemetry";
const int port = 80;

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module


// the setup routine runs once when you press reset:
void setup()
{
  //Initialize serial and wait for port to open:
  //Serial.begin(9600);
  Serial.begin(115200);
  Serial.setTimeout(1500);
  // initialize the digital pin as an output.
  pinMode(LED, OUTPUT);     

  setup_wifi();

  if (!htu.begin())
  {
    Serial.println("Couldn't find temp/humid sensor!");
  }

  //setup_REST();
}

// the loop routine runs over and over again forever:
void loop()
{
  //getData();
  readTempHumid();
  readGasSensor();
  //doBlinkSignature();
  //readParticles();
  Serial.println();
  delay(1000);
}

/*
void readParticles()
{
  if (Serial.find("*")) //start to read when detect 0x42
  {
    Serial.readBytes(buf,LENG);
    if(buf[0] == "M")//0x4d
    {
      if(checkValue(buf,LENG))
      {
        PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 

        Serial.print("PM1.0: ");  
        Serial.print(PM01Value);
        Serial.println("  ug/m3");            
      
        Serial.print("PM2.5: ");  
        Serial.print(PM2_5Value);
        Serial.println("  ug/m3");     
        
        Serial.print("PM1 0: ");  
        Serial.print(PM10Value);
        Serial.println("  ug/m3");   
        Serial.println();      
      }
    }
  }
}
*/

void postData(String channelGuid, int channelTypeID, String captureDate, float value)
{
  POST_client.stop();
  
  if (setup_REST_POST())
  {
    POST_client.println("POST " + url + " HTTP/1.1");
    POST_client.println("Host: weatherbot46.azurewebsites.net");
    POST_client.println("Content-Type: application/json");
    POST_client.println("Connection: close");
    POST_client.println("{");
    POST_client.println("\"channelGuid\": \"" + channelGuid + "\"");
    POST_client.println("\"channelTypeId\": 1");
    POST_client.println("\"captureDate\": \"" + captureDate + "\"");
    POST_client.println("\"value\": 3.14");
    POST_client.println("}");
    POST_client.println();
    
    delay(2000);
    int delayCount = 0;
    while ( (!POST_client.available()) && (delayCount < 10) && (POST_client.connected()) )
    {
      delay(1000);
      delayCount++;
      Serial.print("d-");
    }
    Serial.print("\n|REST_POST: ");
    while (POST_client.available())
    {
      Serial.print(POST_client.read());
    }
    Serial.println("|");
    POST_client.flush();
    POST_client.stop();
  }
}

void POST_writeString(String s)
{
  int len = s.length();
  for (int i = 0; i < len; i++)
  {
    const char c = s.charAt(i);
    int num = 0;
    while ( (num < 1) )//&& (POST_client.connected()) )
    {
      if (!POST_client.connected())
        if (setup_REST_POST())
        {
          num = POST_client.write(c);
          Serial.print(c);
        }
    }
  }
}

void getData()
{
  if (setup_REST_GET())
  {
    // Send GET request
    GET_client.print("GET ");   GET_client.print(url); GET_client.println(" HTTP/1.1"); 
    GET_client.print("Host: "); GET_client.println(hostname);
    GET_client.println("Content-Type: application/json");
    GET_client.println("Cache-Control: no-cache");
    GET_client.println("Connection: close");
    GET_client.println();
    // Wait for response from server
    delay(2000);
    int delayCount = 0;
    while ( (!GET_client.available()) && (delayCount < 10) )
    {
      delay(1000);
      delayCount++;
      Serial.print("d-");
    }
    Serial.print("\n|REST_GET: ");
    while ( GET_client.find("value\":") )
    {
      float value = GET_client.parseFloat();
      Serial.print(value);
      Serial.print(", ");
    }
    Serial.print(" - ");
    while (GET_client.available())
    {
      Serial.print(GET_client.read());
    }
    Serial.println("|");
    GET_client.flush();
    GET_client.stop();
  }
}

void readTempHumid()
{
  htu.begin();

  String channelGuid = "ecb62ea5-a10e-4099-bb29-7bfc03f3b06a";
  int channelTypeID = 1;
  String captureDate = "2018-01-13T23:05:01.083";
  
  const float temperature = htu.readTemperature();
  Serial.print("Temperature: "); Serial.print(temperature);
  //postData(channelGuid, channelTypeID, captureDate, temperature);

  const float humidity = htu.readHumidity();
  Serial.print("\t\tHumidity: "); Serial.println(humidity);
  //postData(channelGuid, channelTypeID, captureDate, humidity);
}

void readGasSensor()
{
  // read the analog in value:
  int sensorValue_A3 = analogRead(A3);           

  // print the results to the serial monitor:
  Serial.print("Gas Sensor: ");
  Serial.println(sensorValue_A3);

  String channelGuid = "ecb62ea5-a10e-4099-bb29-7bfc03f3b06a";
  int channelTypeID = 1;
  String captureDate = "2018-01-13T23:05:01.083";
  //postData(channelGuid, channelTypeID, captureDate, (float)sensorValue_A3);
}

void doBlinkSignature()
{
  Serial.println("Blink");
  digitalWrite(LED, HIGH);    // turn the LED on (HIGH is the voltage level)
  delay(50);                  // short wait
  digitalWrite(LED, LOW);     // turn the LED off by making the voltage LOW
  delay(50);                  // short wait
  digitalWrite(LED, HIGH);    // turn the LED on (HIGH is the voltage level)
  delay(50);                  // short wait
  digitalWrite(LED, LOW);     // turn the LED off by making the voltage LOW
  delay(5000);                // wait for a second
}

bool setup_REST()
{
  const bool GET = setup_REST_GET();
  const bool POST = setup_REST_POST();
  return (GET && POST);
}

bool setup_REST_GET()
{
  Serial.print("Connecting GET to ");
  Serial.print(hostname);
  if ( !GET_client.connect(hostname, port) )
  {
    Serial.println("...GET Connection failed");
    return false;
  }
  else
  {
    Serial.println("...Connected!");
    return true;
  }
}

bool setup_REST_POST()
{
  Serial.print("Connecting POST to ");
  Serial.print(hostname);
  if ( !POST_client.connect(hostname, port) )
  {
    Serial.println("...POST Connection failed");
    return false;
  }
  else
  {
    Serial.println("...Connected!");
    return true;
  }
}

void setup_wifi()
{
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    // print dots while we wait to connect
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");  
  while (WiFi.localIP() == INADDR_NONE)
  {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nIP Address obtained");
  // you're connected now, so print out the status  
  printCurrentNet();
  printWifiData();
}

void printWifiData()
{
  // print your WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
}

void printCurrentNet()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

/*
char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}*/
