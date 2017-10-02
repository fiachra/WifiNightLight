#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "NeoPatterns.h"
#include "NightLight.h"

//Wifi Stuff
#include "wifinfo.h"
//You should create your own version of this file that jsut containts the following:
//const char* ssid = "**********";
//const char* password = "**********";
//Obviously replace stars with your own ssid and password information





int phase = 0;
int phaseTime = 5000;
int lastPhaseShift = 0;
//web server
ESP8266WebServer server(80);

uint32_t Colour(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

void CycleComplete();

NightLight nightLight(&CycleComplete);

void CycleComplete() {
  nightLight.completeCallback();
}
/****************************************************************/

void setup(void)
{
  Serial.begin(115200);
  // Connect to WiFi network
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  //    colorWipe(strip.Color(255, 0, 0), 50); // Red

  WiFi.begin(ssid, password);


  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //    colorWipe(strip.Color(0, 0, 255), 50); // Blue

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  float nightBrightness = 0.8;

  int nightR = 0 * nightBrightness;
  int nightG = 0 * nightBrightness;
  int nightB = 255 * nightBrightness;

  int dayR = 255;
  int dayG = 128;
  int dayB = 0;

  //nightLight.setDayColour(Colour(dayR,dayG,dayB));
  //nightLight.setNightColour(Colour(nightR,nightG,nightB));

  nightLight.begin();

  lastPhaseShift = millis();

  server.on("/", []() {
    server.send(200, "text/plain", nightLight.getStatus());
  });

  server.on("/alarm", []() {
    server.send(200, "text/plain", "Alarm!!");
    nightLight.triggerAlarm();
  });

  server.on("/off", []() {
    server.send(200, "text/plain", "Alarm!!");
    nightLight.setOn(false);
  });

  server.on("/on", []() {
    server.send(200, "text/plain", "Alarm!!");
    nightLight.setOn(true);
  });

  server.on("/setTimes", handleTime);
  server.on("/setMorn", handleMorningCol);
  server.on("/setseNight", handleNightCol);

  server.begin();
  
}
/*
/"setTimes?mh=19&mm=25&nh=19&nm=30
/setMorn?r=255&g=128&b=0
/setNight?r=0&g=0&b=204"
*/

void handleTime() {
  int  mornH = 7;
  int  mornM = 15;
  int  nighH = 19;
  int  nighM = 30;
  

  if (!server.hasArg("mh")) return returnFail("BAD ARGS");
  mornH = (server.arg("mh").toInt());
  if (!server.hasArg("mm")) return returnFail("BAD ARGS");
  mornM = (server.arg("mm").toInt());
  if (!server.hasArg("nh")) return returnFail("BAD ARGS");
  nighH = server.arg("nh").toInt();
   if (!server.hasArg("nm")) return returnFail("BAD ARGS");
  nighM = server.arg("nm").toInt();

  server.send(200, "text/plain",  "Good Time");

  nightLight.setMorningTime((mornH*3600)+(mornM*60));
  nightLight.setNightTime((nighH*3600)+(nighM*60));


  Serial.println("Time Set");

}

void handleMorningCol() {
  int  r = 0;
  int  g = 0;
  int  b = 0;
 
  if (!server.hasArg("r")) return returnFail("BAD ARGS");
  r = (server.arg("r").toInt());
  if (!server.hasArg("g")) return returnFail("BAD ARGS");
  g = (server.arg("g").toInt());
  if (!server.hasArg("b")) return returnFail("BAD ARGS");
  b = server.arg("b").toInt();

  server.send(200, "text/plain",  "Good Colour");

  nightLight.setDayColour(Colour(r,g,b));

  Serial.println("Time Set");

}

void handleNightCol() {
  int  r = 0;
  int  g = 0;
  int  b = 0;
 
  if (!server.hasArg("r")) return returnFail("BAD ARGS");
  r = (server.arg("r").toInt());
  if (!server.hasArg("g")) return returnFail("BAD ARGS");
  g = (server.arg("g").toInt());
  if (!server.hasArg("b")) return returnFail("BAD ARGS");
  b = server.arg("b").toInt();

  server.send(200, "text/plain",  "Good Colour");

  nightLight.setNightColour(Colour(r,g,b));

  Serial.println("Time Set");

}



void returnFail(String msg)
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
  Serial.println("TimeVal - Bad Args");
}


void loop(void)
{
  static int counter = 0;
  
  nightLight.update();
  server.handleClient();
  long currTime = millis();
  /*
  if((currTime - lastPhaseShift) > phaseTime)
  {
    lastPhaseShift = currTime;
    if(counter == 0){
      nightLight.setDay();
      Serial.println("SetDay");
    }
    else if(counter == 1){
      nightLight.setNight();
      Serial.println("SetNight");
    }
    else if(counter == 2){
      nightLight.triggerAlarm();
      Serial.println("SetAlarm");
    }
    counter ++;
    counter = counter % 3;
  }
  */
}


