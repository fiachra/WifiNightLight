#ifndef NIGHTLIGHT_H
#define NIGHTLIGHT_H

#include "NeoPatterns.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

class NightLight
{
public:
	
	NightLight(void (*callback)());
	~NightLight();

	void setMorningTime(int morningTime);
	void setNightTime(int nightTime);

	void setDayColour(uint32_t dayColour);
	void setNightColour(uint32_t nightColour);
  	void setOn(bool on);

	void begin();

	void setDay();
	void setNight();
	void triggerAlarm();
  

  	String getStatus();


	void completeCallback();
	void update();

	const int iPin = 2; 
private:
	uint32_t m_nightColour;
	uint32_t m_dayColour;
	
	bool m_alarmTriggered;
	bool m_alarmComplete;
	long m_alarmStartTime;
	int MORNING_ALARM_DURATION = 15000;

	int TIME_CHECK_UPDATE = 5000;
	long m_lastUpdateTime;

	int m_morningTime;
	int m_nightTime;

	bool m_isNight;
  	bool m_on;

	NeoPatterns *m_lights;
	WiFiUDP m_ntpUDP;
	NTPClient *m_timeClient;

	int getCurrentClockTime();
	bool getIsNight();

  	bool loadConfigData();
  	void saveConfigData();
  


};

#endif
