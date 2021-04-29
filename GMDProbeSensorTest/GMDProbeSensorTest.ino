#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#include "CytronMotorDriver.h"
#define DHTTYPE DHT11
#define ECHO_TO_SERIAL 1   //Sends datalogging to serial if 1, nothing if 0
#define LOG_INTERVAL 10000 //milliseconds between entries (6 minutes = 360000)

//Pin assignments
const int soilMoisturePin[5] = {A1, A2, A3, A4};
const int dhtPin = 2;
const int sunlightPin = A5;

DHT dht(dhtPin, DHTTYPE);

//Initialize variables
float soilMoistureRaw = 0; //Raw analog input of soil moisture sensor (volts)
float soilMoisture[5];     //Scaled value of volumetric water content in soil (percent)
float soilCalibration[5] = {3.887, 3.642, 3.642, 3.807};
int plantIDs[5] = {30, 31, 33, 34};
float humidity = 0;        //Relative humidity (%)
float airTemp = 0;         //Air temp (degrees F)
float sunlight1 = 0;
float sunlight2 = 0;
float sunlight = 0; 	   //Sunlight illumination (lux)

//WiFi login credentials
char ssid[] = "test"; 
char pass[] = "";
char server[] = "71142021.000webhostapp.com";
String postData;
int status = WL_IDLE_STATUS;
WiFiClient client;

//LCD and motor variables
CytronMD motor(PWM_DIR, 3, 4);  // PWM = Pin 3, DIR = Pin 4.
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  
int x = analogRead (A0); 

int mode=0;
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  while(1);
}

int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 250)  return btnUP;
 if (adc_key_in < 450)  return btnDOWN;
 if (adc_key_in < 650)  return btnLEFT;
 if (adc_key_in < 850)  return btnSELECT;

 // For V1.0 comment the other threshold and use the one below:
/*
 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 195)  return btnUP;
 if (adc_key_in < 380)  return btnDOWN;
 if (adc_key_in < 555)  return btnLEFT;
 if (adc_key_in < 790)  return btnSELECT;
*/
 return btnNONE;  // when all others fail, return this...
}

void lcdButtons() {
 lcd.setCursor(0,0);
 lcd_key = read_LCD_buttons();

 switch (lcd_key){
   case btnRIGHT:
     {
     lcd.print("RIGHT ");
     if(mode==0){
      lcd.print ("Solenoid ON");
      motor.setSpeed(-255);
      delay(1000);
     }
     if(mode==1){
      lcd.print ("Mositure Sensors ON");
      digitalWrite(D1, HIGH);
      digitalWrite(D11, HIGH);
      digitalWrite(D12, HIGH);
      digitalWrite(D13, HIGH);
     }
     break;
     }
   case btnLEFT:
     {
     lcd.print("LEFT   ");
     if(mode==0){
      lcd.print ("Solenoid OFF");
      motor.setSpeed(255);
      delay(1000);
     }
     if(mode==1){
      lcd.print ("Mositure Sensors OFF");
      digitalWrite(D1, LOW);
      digitalWrite(D11, LOW);
      digitalWrite(D12, LOW);
      digitalWrite(D13, LOW);
     }
     break;
     }
   case btnUP:
     {
     lcd.print("Solenoid");
     mode=0;
     break;
     }
   case btnDOWN:
     {
     lcd.print("Mositure Sensors");
     mode=1;
     break;
     }
   case btnSELECT:
     {
     lcd.print("SELECT");
     break;
     }
     case btnNONE:
     {
     lcd.print("NONE  ");
     break;
     }
 }
}

void setup() {
  
  //Initialize serial connection
  Serial.begin(38400); //Just for testing
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
  
  //LCD and motor initialization
  pinMode(D1,OUTPUT);
  pinMode(D11,OUTPUT);
  pinMode(D12,OUTPUT);
  pinMode(D13,OUTPUT);   
  CytronMD motor(PWM_DIR, A0, A1); 
  lcd.begin(16, 2);           
  lcd.setCursor(0,0);
  attachInterrupt(digitalPinToInterrupt(A0), lcdButtons, CHANGE);
  lcd.print("Push the buttons");

#if ECHO_TO_SERIAL
  Serial.println("Air Temp (F),Soil Moisture Content (%),Relative Humidity (%),Sunlight Illumination (lux)");
#endif ECHO_TO_SERIAL // attempt to write out the header to the console
}

void loop()
{

  //Wait for reading interval
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));
 
  for (int i = 0; i < 5; i++)
  {
  //Volumetric Water Content is a piecewise function of the voltage from the sensor
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
  if (client.connect(server, 80)) {
  Serial.println("Uploading to database...");
  postData = "plantId1=";
  postData.concat(plantIDs[0]);
  postData.concat("plantId2=");
  postData.concat(plantIDs[1]);
  postData.concat("plantId3=");
  postData.concat(plantIDs[2]);
  postData.concat("plantId4=");
  postData.concat(plantIDs[3]);
  postData.concat("&temperature=");
  postData.concat(airTemp);
  postData.concat("&moisture1=");
  postData.concat(soilMoisture[0]);
  postData.concat("&moisture2=");
  postData.concat(soilMoisture[1]);
  postData.concat("&moisture3=");
  postData.concat(soilMoisture[2]);
  postData.concat("&moisture4=");
  postData.concat(soilMoisture[3]);
  postData.concat("&humidity=");
  postData.concat(humidity);
  postData.concat("&sunlight=");
  postData.concat(sunlight);
  client.println("POST /uploadReadings.php HTTP/1.1");
  client.println("Host: 71142021.000webhostapp.com");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(postData.length());
  client.println();
  client.print(postData);
  delay(5000);
}
if (client.connected()) {
  Serial.println("Upload complete");
  client.stop();
}
delay(1000);
  
#if ECHO_TO_SERIAL
  Serial.println();
#endif
  delay(50);
}
