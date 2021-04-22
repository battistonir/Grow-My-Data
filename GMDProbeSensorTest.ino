#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include "RTClib.h"
#define DHTTYPE DHT11
#define ECHO_TO_SERIAL 1   //Sends datalogging to serial if 1, nothing if 0
#define LOG_INTERVAL 10000 //milliseconds between entries (6 minutes = 360000)

//Pin assignments
const int soilMoisturePin[5] = {A1, A2, A3, A4};
const int dhtPin = 2;
const int sunlightPin = A0;

DHT dht(dhtPin, DHTTYPE);
RTC_DS1307 rtc;

//Initialize variables
float soilMoistureRaw = 0; //Raw analog input of soil moisture sensor (volts)
float soilMoisture[5];    //Scaled value of volumetric water content in soil (percent)
float humidity = 0;        //Relative humidity (%)
float airTemp = 0;         //Air temp (degrees F)
float sunlight = 0; 	   //Sunlight illumination (lux)
DateTime now;

//WiFi login credentials
char ssid[] = "test"; 
char pass[] = "";
char server[] = "71142021.000webhostapp.com";
String postData;
int status = WL_IDLE_STATUS;
WiFiClient client;

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
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
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

#if ECHO_TO_SERIAL
  Serial.println("Air Temp (F),Soil Moisture Content (%),Relative Humidity (%),Sunlight Illumination (lux)");
#endif ECHO_TO_SERIAL // attempt to write out the header to the console
  now = rtc.now();
}

void loop()
{

  //Wait for reading interval
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));

  now = rtc.now();

  //Collect raw moisture from voltage
  soilMoistureRaw = analogRead(soilMoisturePin) * (3.8874 / 1024);
  delay(20);
  
  //Volumetric Water Content is a piecewise function of the voltage from the sensor
  for (int i = 0; i < 5; i++)
  {
	soilMoistureRaw = analogRead(soilMoisturePin[i]) * (3.8874 / 1024);
	delay(20);
  
	//Volumetric Water Content is a piecewise function of the voltage from the sensor
	if (soilMoistureRaw <= 1.1)
	{
		soilMoisture = (10 * soilMoistureRaw) - 1;
	}
	else if (soilMoistureRaw > 1.1 && soilMoistureRaw <= 1.3)
	{
		soilMoisture = (25 * soilMoistureRaw) - 17.5;
	}
	else if (soilMoistureRaw > 1.3 && soilMoistureRaw <= 1.82)
	{
		soilMoisture = (48.08 * soilMoistureRaw) - 47.5;
	}
	else if (soilMoistureRaw > 1.82 && soilMoistureRaw < 2.2)
	{
    soilMoisture = (26.32 * soilMoistureRaw) - 7.89;
	}
	else {
		soilMoisture = (62.5 * soilMoistureRaw) - 87.5;
	}

	if (soilMoisture < 0)
	{
		soilMoisture = 0;
	}
	else if (soilMoisture > 100)
	{
		soilMoisture = 100;
	}
	delay(20);
  }

  //Collect humidity
  humidity = dht.readHumidity();
  delay(20);

  //Collect temperature (true = Fahrenheit)
  airTemp = dht.readTemperature(true);
  delay(20);
  
  //Collect sunlight with a rough conversion
  sunlight = pow(((((150 * 3.3)/(analogRead(sunlightPin)*(3.3/1024))) - 150) / 70000),-1.25);
  delay(20);

  //Print measurements to serial monitor
#if ECHO_TO_SERIAL
  Serial.print(airTemp);
  Serial.print(",");
  Serial.print(soilMoisture[0]);
  Serial.print(",");
  Serial.print(soilMoisture[1]);
  Serial.print(",");
  Serial.print(soilMoisture[2]);
  Serial.print(",");
  Serial.print(soilMoisture[3]);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
  Serial.print(sunlight);
  Serial.print(",");
#endif
  if (client.connect(server, 80)) {
	postData = "plantId=30&temperature=";
	postData.concat(airTemp);
	postData.concat("&moisture=");
	postData.concat(soilMoisture[0]);
	postData.concat("&humidity=");
	postData.concat(humidity);
	postData.concat("&sunlight=");
	postData.concat(sunlight);
    client.println("POST /setPlantTraits.php HTTP/1.1");
    client.println("Host: 71142021.000webhostapp.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.print(postData);
	delay(1000);
  }
  if (client.connected()) {
    client.stop();
  }
  delay(100);
  if (client.connect(server, 80)) {
	postData = "plantId=31&temperature=";
	postData.concat(airTemp);
	postData.concat("&moisture=");
	postData.concat(soilMoisture[1]);
	postData.concat("&humidity=");
	postData.concat(humidity);
	postData.concat("&sunlight=");
	postData.concat(sunlight);
    client.println("POST /setPlantTraits.php HTTP/1.1");
    client.println("Host: 71142021.000webhostapp.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.print(postData);
	delay(1000);
  }
  if (client.connected()) {
    client.stop();
  }
  delay(100);
  if (client.connect(server, 80)) {
	postData = "plantId=33&temperature=";
	postData.concat(airTemp);
	postData.concat("&moisture=");
	postData.concat(soilMoisture[2]);
	postData.concat("&humidity=");
	postData.concat(humidity);
	postData.concat("&sunlight=");
	postData.concat(sunlight);
    client.println("POST /setPlantTraits.php HTTP/1.1");
    client.println("Host: 71142021.000webhostapp.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.print(postData);
	delay(1000);
  }
  if (client.connected()) {
    client.stop();
  }
  delay(100);
  if (client.connect(server, 80)) {
	postData = "plantId=34&temperature=";
	postData.concat(airTemp);
	postData.concat("&moisture=");
	postData.concat(soilMoisture[3]);
	postData.concat("&humidity=");
	postData.concat(humidity);
	postData.concat("&sunlight=");
	postData.concat(sunlight);
    client.println("POST /setPlantTraits.php HTTP/1.1");
    client.println("Host: 71142021.000webhostapp.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postData.length());
    client.println();
    client.print(postData);
	delay(1000);
  }
  if (client.connected()) {
    client.stop();
  }
  
#if ECHO_TO_SERIAL
  Serial.println();
#endif
  delay(50);
}