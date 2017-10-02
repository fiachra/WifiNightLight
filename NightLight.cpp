#include "NightLight.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "FS.h"


NightLight::NightLight(void (*callback)())
{
	m_lights = new NeoPatterns(32, iPin, NEO_GRB + NEO_KHZ800, callback);
	m_timeClient  = new NTPClient(m_ntpUDP, "europe.pool.ntp.org", 3600, 60000);
	m_isNight = false;
	m_morningTime = (7*3600) + (15*60);
	m_nightTime = (19*3600) + (0*60);
  m_dayColour = 16744448;
  m_nightColour = 204; 
  Serial.println("NL Costructer");
  m_on = true;

  SPIFFS.begin();
}

NightLight::~NightLight()
{
	delete m_lights;
	delete m_timeClient;
}

void NightLight::begin()
{
	m_timeClient->begin();
	delay(1000);
	m_timeClient->update();

  m_lights->begin();
  m_lights->TheaterChase(m_lights->Color(255, 255, 0), m_lights->Color(0, 0, 50), 100);
  m_lights->Update();

  loadConfigData();
	
	m_isNight = getIsNight();


	if(m_isNight)
		setNight();
	else
		setDay();   
}

String NightLight::getStatus(){

  String message = "Current Time:";
  message += m_timeClient->getFormattedTime();
  message += " -- ";
  message += String(getCurrentClockTime());
  message += "\nStatus:";
  message += m_on ? "On" : "Off";
  message += "\nMorning Start:";
  message += String(m_morningTime);
  message += "\n";
  message += String(m_morningTime/3600);
  message += ":";
  message += String((m_morningTime%3600)/60);
  message += "\nNight Start:";
  message += String(m_nightTime);
  message += "\n";
  message += String(m_nightTime/3600);
  message += ":";
  message += String((m_nightTime%3600)/60);
  message += "\nDay Colour:";
  message += String(m_dayColour);
   message += "\nNight Colour:";
  message += String(m_nightColour);
  message += "\nsetTimes?mh=19&mm=25&nh=19&nm=30\n/setMorn?r=255&g=128&b=0\n/setNight?r=0&g=0&b=204";

  return message;
}

void  NightLight::setOn(bool on)
{
  m_on = on;

  if(!m_on){
    m_lights->ColorSet(0);
  }else
  {
    if(m_isNight)
        setNight();
      else
        setDay();
  }
  
}
void NightLight::setMorningTime(int morningTime)
{
	m_morningTime = morningTime;
}
void NightLight::setNightTime(int nightTime)
{
	m_nightTime = nightTime;
    saveConfigData();
}

void NightLight::setDayColour(uint32_t dayColour)
{
	m_dayColour = dayColour;
  saveConfigData();
}
void NightLight::setNightColour(uint32_t nightColour)
{
	m_nightColour = nightColour;
  saveConfigData();
}

void NightLight::setDay()
{
  if(m_on)
	  m_lights->ColorWipe(m_dayColour, 50);
 
  Serial.println(m_timeClient->getFormattedTime());
}
void NightLight::setNight()
{
	if(m_on)
    m_lights->ColorWipe(m_nightColour, 50);
 
  Serial.println(m_timeClient->getFormattedTime());
  
}
void NightLight::triggerAlarm()
{
  Serial.println("TriggerAlarm");
	m_alarmTriggered = true;
	m_alarmStartTime = millis();
	if(m_on)
    m_lights->TheaterChase(m_dayColour, m_nightColour, 100);
    
}

void NightLight::completeCallback()
{
	m_lights->Reverse();
}

int NightLight::getCurrentClockTime()
{
	return (m_timeClient->getHours() * 3600) + (m_timeClient->getMinutes() * 60) + (m_timeClient->getSeconds());
}

void NightLight::update()
{
	m_lights->Update();
	m_timeClient->update();
	long currTime = millis();

	if(m_alarmTriggered){
		if((currTime - m_alarmStartTime)>MORNING_ALARM_DURATION){
      Serial.println("TriggerAlarmComplete");
			m_alarmTriggered = false;
			if(m_isNight)
				setNight();
			else
				setDay();  
		}
	}

	if((currTime - m_lastUpdateTime) > TIME_CHECK_UPDATE)
	{
		m_lastUpdateTime = currTime;

		bool isNight = getIsNight();

		if(m_isNight != isNight)
		{
			m_isNight = isNight;
      Serial.println("TriggerAlarmUpdate");
			triggerAlarm();
		}
	}
}

bool NightLight::getIsNight()
{
	int currentClockTime = getCurrentClockTime();
	bool isNight;

	Serial.println(currentClockTime);

	if(currentClockTime > m_morningTime && currentClockTime < m_nightTime)
		isNight = false;
	else
		isNight = true;

	return isNight;
}

bool NightLight::loadConfigData()
{
  Serial.println("Loading config file");
  File configFile = SPIFFS.open("/nightlightconfig.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    saveConfigData();
    return false;
  }

  size_t size = configFile.size();
  if (size > 4096) {
    Serial.println("Config file size is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<2000> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  m_dayColour = json["morningcolour"];
  m_nightColour = json["nightcolour"];
  m_morningTime = json["daytimestart"];
  m_nightTime = json["nighttimestart"];
  
}
void NightLight::saveConfigData()
{
  Serial.println("Saving config file");
  StaticJsonBuffer<2000> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  
  root["morningcolour"] = m_dayColour;
  root["nightcolour"] = m_nightColour;
  root["daytimestart"] = m_morningTime;
  root["nighttimestart"] = m_nightTime;
  
  File configFile = SPIFFS.open("/nightlightconfig.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  root.printTo(configFile);
  
}
  

