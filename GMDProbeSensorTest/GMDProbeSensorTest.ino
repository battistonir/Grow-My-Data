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

//Initialize variables
float soilMoistureRaw = 0; //Raw analog input of soil moisture sensor (volts)
float soilMoisture[5];     //Scaled value of volumetric water content in soil (percent)
float soilCalibration[5] = {3.887, 3.642, 3.642, 3.807};
int plantIDs[5] = {30, 31, 33, 34};
float humidity = 0;        //Relative humidity (%)
float airTemp = 0;         //Air temp (degrees F)
float sunlight = 0; 	   //Sunlight illumination (lux)

//WiFi login credentials
char ssid[] = "Hacienda"; 
char pass[] = "wifidecasa85";
char server[] = "71142021.000webhostapp.com";
String postData;
int status = WL_IDLE_STATUS;
WiFiClient client;

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
  Serial.println("Success!");
  analogReference(EXTERNAL); //Sets the max voltage from analog inputs to whatever is connected to the Aref pin (should be 3.3v)
  //Establish connection with DHT sensor
  dht.begin();
  
  //Establish connection with real time clock
  Wire.begin();

#if ECHO_TO_SERIAL
  Serial.println("Air Temp (F),Soil Moisture Content (%),Relative Humidity (%),Sunlight Illumination (lux)");
#endif ECHO_TO_SERIAL // attempt to write out the header to the console
}

void loop()
{

  //Wait for reading interval
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));

  //Volumetric Water Content is a piecewise function of the voltage from the sensor
  for (int i = 0; i < 5; i++)
  {
	soilMoistureRaw = analogRead(soilMoisturePin[i]) * (soilCalibration[i] / 1024);
	delay(20);
  
	//Volumetric Water Content is a piecewise function of the voltage from the sensor
	if (soilMoistureRaw <= 1.1)
	{
		soilMoisture[i] = (10 * soilMoistureRaw) - 1;
	}
	else if (soilMoistureRaw > 1.1 && soilMoistureRaw <= 1.3)
	{
		soilMoisture[i] = (25 * soilMoistureRaw) - 17.5;
	}
	else if (soilMoistureRaw > 1.3 && soilMoistureRaw <= 1.82)
	{
		soilMoisture[i] = (48.08 * soilMoistureRaw) - 47.5;
	}
	else if (soilMoistureRaw > 1.82 && soilMoistureRaw < 2.2)
	{
    soilMoisture[i] = (26.32 * soilMoistureRaw) - 7.89;
	}
	else {
		soilMoisture[i] = (62.5 * soilMoistureRaw) - 87.5;
	}

	if (soilMoisture[i] < 0)
	{
		soilMoisture[i] = 0;
	}
	else if (soilMoisture[i] > 100)
	{
		soilMoisture[i] = 100;
	}
	delay(1000);
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
  for (int i = 0; i < 5; i++)
  {
	if (client.connect(server, 80)) {
		postData = "plantId=";
		postData.concat(plantIDs[i]);
		postData.concat("&temperature=");
		postData.concat(airTemp);
		postData.concat("&moisture=");
		postData.concat(soilMoisture[i]);
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
	delay(5000);
  }
  
#if ECHO_TO_SERIAL
  Serial.println();
#endif
  delay(50);
}
