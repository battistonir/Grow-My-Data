/*
Automatic Garden Waterer and Datalogger
Original by Grady Hillhouse (MIT License)
*/

#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#define DHTTYPE DHT11
#define ECHO_TO_SERIAL 1 //Sends datalogging to serial if 1, nothing if 0
#define LOG_INTERVAL 10000 //milliseconds between entries (6 minutes = 360000)

const int soilMoisturePin = A1;
const int dhtPin = 2;

DHT dht(dhtPin, DHTTYPE);
RTC_DS1307 rtc;

float soilMoistureRaw = 0; //Raw analog input of soil moisture sensor (volts)
float soilMoisture; //Scaled value of volumetric water content in soil (percent)
float humidity = 0; //Relative humidity (%)
float airTemp = 0; //Air temp (degrees F)
DateTime now;

/*
Soil Moisture Reference
Air = 0%
Really dry soil = 10%
Probably as low as you'd want = 20%
Well watered = 50%
Cup of water = 100%
*/

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  while(1);
}

void setup() {
  
  //Initialize serial connection
  Serial.begin(9600); //Just for testing
  Serial.println("Initializing...");
  //pinMode(solenoidPin, OUTPUT); //solenoid pin
  //digitalWrite(solenoidPin, LOW); //Make sure the valve is off
  analogReference(EXTERNAL); //Sets the max voltage from analog inputs to whatever is connected to the Aref pin (should be 3.3v)
  
  //Establish connection with DHT sensor
  dht.begin();
  
  //Establish connection with real time clock
  Wire.begin();
  if (!rtc.begin()) {
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  
  //Set the time and date on the real time clock if necessary
  if (! rtc.isrunning()) {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
  }
  
#if ECHO_TO_SERIAL
  Serial.println("Unix Time (s),Date,Soil Temp (F),Air Temp (F),Soil Moisture Content (%),Relative Humidity (%)");
#endif ECHO_TO_SERIAL// attempt to write out the header to the console
  now = rtc.now();
}

void loop() {
    
  //delay software
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  now = rtc.now();
  
  // log time
 #if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 2000
  Serial.print(",");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print(",");
#endif //ECHO_TO_SERIAL
  
  //Collect Variables
  //for (int i = 0; i < 5; i++)
  //{
	//soilMoistureRaw = analogRead(soilMoisturePin[i])*(3.3/1024);
	//delay(20);
soilMoistureRaw = analogRead(soilMoisturePin)*(3.3/1024);
 delay(20);

  
	//Volumetric Water Content is a piecewise function of the voltage from the sensor
	if (soilMoistureRaw < 1.1) {
		soilMoisture = (10 * soilMoistureRaw) - 1;
	}
	else if (soilMoistureRaw < 1.3) {
		soilMoisture = (25 * soilMoistureRaw) - 17.5;
	}
	else if (soilMoistureRaw < 1.82) {
		soilMoisture = (48.08 * soilMoistureRaw) - 47.5;
	}
	else if (soilMoistureRaw < 2.2) {
		soilMoisture = (26.32 * soilMoistureRaw) - 7.89;
	}
	else {
		soilMoisture = (62.5 * soilMoistureRaw) - 87.5;
	}
	delay(20);
  }
    
  humidity = dht.readHumidity();
  delay(20);
  
  airTemp = dht.readTemperature(true);
  delay(20);
  
  //This is a rough conversion that I tried to calibrate using a flashlight of a "known" brightness
  delay(20);

#if ECHO_TO_SERIAL
  Serial.print(airTemp);
  Serial.print(",");
  Serial.print(soilMoisture);
  Serial.print(",");
  //Serial.print(soilMoisture[1]);
  Serial.print(",");
  //Serial.print(soilMoisture[2]);
  Serial.print(",");
  //Serial.print(soilMoisture[3]);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
#endif
  
#if ECHO_TO_SERIAL
  Serial.println();
#endif
  delay(50);
}
