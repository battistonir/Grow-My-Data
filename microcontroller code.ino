/*
Automatic Garden Waterer and Datalogger

Grady Hillhouse (March 2015)
*/

#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include "SD.h"
#define DHTTYPE DHT22
#define ECHO_TO_SERIAL 1    //Sends datalogging to serial if 1, nothing if 0
#define LOG_INTERVAL 360000 //milliseconds between entries (6 minutes = 360000)

float readVH400(int analogPin)
{
  // This function returns Volumetric Water Content by converting the analogPin value to voltage
  // and then converting voltage to VWC using the piecewise regressions provided by the manufacturer
  // at http://www.vegetronix.com/Products/VH400/VH400-Piecewise-Curve.phtml

  // NOTE: You need to set analogPin to input in your setup block
  //   ex. pinMode(<analogPin>, INPUT);
  //   replace <analogPin> with the number of the pin you're going to read from.

  // Read value and convert to voltage
  int sensor1DN = analogRead(analogPin);
  float sensorVoltage = sensor1DN * (3.0 / 1023.0);
  float VWC;

  // Calculate VWC
  if (sensorVoltage <= 1.1)
  {
    VWC = 10 * sensorVoltage - 1;
  }
  else if (sensorVoltage > 1.1 && sensorVoltage <= 1.3)
  {
    VWC = 25 * sensorVoltage - 17.5;
  }
  else if (sensorVoltage > 1.3 && sensorVoltage <= 1.82)
  {
    VWC = 48.08 * sensorVoltage - 47.5;
  }
  else if (sensorVoltage > 1.82)
  {
    VWC = 26.32 * sensorVoltage - 7.89;
  }
  return (VWC);
}
struct VH400
{
  double analogValue;
  double analogValue_sd;
  double voltage;
  double voltage_sd;
  double VWC;
  double VWC_sd;
};

struct VH400 readVH400_wStats(int analogPin, int nMeasurements = 100, int delayBetweenMeasurements = 50)
{
  // This variant calculates the mean and standard deviation of 100 measurements over 5 seconds.
  // It reports mean and standard deviation for the analog value, voltage, and WVC.

  // This function returns Volumetric Water Content by converting the analogPin value to voltage
  // and then converting voltage to VWC using the piecewise regressions provided by the manufacturer
  // at http://www.vegetronix.com/Products/VH400/VH400-Piecewise-Curve.phtml

  // NOTE: You need to set analogPin to input in your setup block
  //   ex. pinMode(<analogPin>, INPUT);
  //   replace <analogPin> with the number of the pin you're going to read from.

  struct VH400 result;

  // Sums for calculating statistics
  int sensorDNsum = 0;
  double sensorVoltageSum = 0.0;
  double sensorVWCSum = 0.0;
  double sqDevSum_DN = 0.0;
  double sqDevSum_volts = 0.0;
  double sqDevSum_VWC = 0.0;

  // Arrays to hold multiple measurements
  int sensorDNs[nMeasurements];
  double sensorVoltages[nMeasurements];
  double sensorVWCs[nMeasurements];

  // Make measurements and add to arrays
  for (int i = 0; i < nMeasurements; i++)
  {
    // Read value and convert to voltage
    int sensorDN = analogRead(analogPin);
    double sensorVoltage = sensorDN * (3.0 / 1023.0);

    // Calculate VWC
    float VWC;
    if (sensorVoltage <= 1.1)
    {
      VWC = 10 * sensorVoltage - 1;
    }
    else if (sensorVoltage > 1.1 && sensorVoltage <= 1.3)
    {
      VWC = 25 * sensorVoltage - 17.5;
    }
    else if (sensorVoltage > 1.3 && sensorVoltage <= 1.82)
    {
      VWC = 48.08 * sensorVoltage - 47.5;
    }
    else if (sensorVoltage > 1.82)
    {
      VWC = 26.32 * sensorVoltage - 7.89;
    }

    // Add to statistics sums
    sensorDNsum += sensorDN;
    sensorVoltageSum += sensorVoltage;
    sensorVWCSum += VWC;

    // Add to arrays
    sensorDNs[i] = sensorDN;
    sensorVoltages[i] = sensorVoltage;
    sensorVWCs[i] = VWC;

    // Wait for next measurement
    delay(delayBetweenMeasurements);
  }

  // Calculate means
  double DN_mean = double(sensorDNsum) / double(nMeasurements);
  double volts_mean = sensorVoltageSum / double(nMeasurements);
  double VWC_mean = sensorVWCSum / double(nMeasurements);

  // Loop back through to calculate SD
  for (int i = 0; i < nMeasurements; i++)
  {
    sqDevSum_DN += pow((DN_mean - double(sensorDNs[i])), 2);
    sqDevSum_volts += pow((volts_mean - double(sensorVoltages[i])), 2);
    sqDevSum_VWC += pow((VWC_mean - double(sensorVWCs[i])), 2);
  }
  double DN_stDev = sqrt(sqDevSum_DN / double(nMeasurements));
  double volts_stDev = sqrt(sqDevSum_volts / double(nMeasurements));
  double VWC_stDev = sqrt(sqDevSum_VWC / double(nMeasurements));

  // Setup the output struct
  result.analogValue = DN_mean;
  result.analogValue_sd = DN_stDev;
  result.voltage = volts_mean;
  result.voltage_sd = volts_stDev;
  result.VWC = VWC_mean;
  result.VWC_sd = VWC_stDev;

  // Return the result
  return (result);
}

const int soilZone1Pin = A1;
const int soilMoisturePin = A1;
const int sunlightPin = A0;
const int dhtPin = 2;
const int chipSelect = 10;
const int LEDPinGreen = 6;
const int LEDPinRed = 7;
const int solenoidPin = 3;
const int wateringTime = 600000;    //Set the watering time (10 min for a start)
const float wateringThreshold = 15; //Value below which the garden gets watered

DHT dht(dhtPin, DHTTYPE);
RTC_DS1307 rtc;

float soilTemp = 0;        //Scaled value of soil temp (degrees F)
float soilMoistureRaw = 0; //Raw analog input of soil moisture sensor (volts)
float soilMoisture = 0;    //Scaled value of volumetric water content in soil (percent)
float humidity = 0;        //Relative humidity (%)
float airTemp = 0;         //Air temp (degrees F)
float heatIndex = 0;       //Heat index (degrees F)
float sunlight = 0;        //Sunlight illumination in lux
bool watering = false;
bool wateredToday = false;
DateTime now;
File logfile;

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

  // red LED indicates error
  digitalWrite(LEDPinRed, HIGH);

  while (1)
    ;
}

void setup()
{

  //Initialize serial connection
  Serial.begin(9600); //Just for testing
  Serial.println("Initializing SD card...");

  pinMode(chipSelect, OUTPUT);    //Pin for writing to SD card
  pinMode(LEDPinGreen, OUTPUT);   //LED green pint
  pinMode(LEDPinRed, OUTPUT);     //LED red pin
  pinMode(solenoidPin, OUTPUT);   //solenoid pin
  digitalWrite(solenoidPin, LOW); //Make sure the valve is off
  analogReference(EXTERNAL);      //Sets the max voltage from analog inputs to whatever is connected to the Aref pin (should be 3.3v)

  //Establish connection with DHT sensor
  dht.begin();

  //Establish connection with real time clock
  Wire.begin();
  if (!rtc.begin())
  {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif //ECHO_TO_SERIAL
  }

  //Set the time and date on the real time clock if necessary
  if (!rtc.isrunning())
  {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //Check if SD card is present and can be initialized
  if (!SD.begin(chipSelect))
  {
    error("Card failed, or not present");
  }

  Serial.println("Card initialized.");

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++)
  {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (!SD.exists(filename))
    {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break; // leave the loop!
    }
  }

  if (!logfile)
  {
    error("couldnt create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);

  logfile.println("Unix Time (s),Date,Soil Temp (F),Air Temp (F),Soil Moisture Content (%),Relative Humidity (%),Heat Index (F),Sunlight Illumination (lux),Watering?"); //HEADER
#if ECHO_TO_SERIAL
  Serial.println("Unix Time (s),Date,Soil Temp (F),Air Temp (F),Soil Moisture Content (%),Relative Humidity (%),Heat Index (F),Sunlight Illumination (lux),Watering?");
#endif ECHO_TO_SERIAL // attempt to write out the header to the file

  now = rtc.now();
}

void loop()
{

  //delay software
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));

  //Three blinks means start of new cycle
  digitalWrite(LEDPinGreen, HIGH);
  delay(150);
  digitalWrite(LEDPinGreen, LOW);
  delay(150);
  digitalWrite(LEDPinGreen, HIGH);
  delay(150);
  digitalWrite(LEDPinGreen, LOW);
  delay(150);
  digitalWrite(LEDPinGreen, HIGH);
  delay(150);
  digitalWrite(LEDPinGreen, LOW);

  //Reset wateredToday variable if it's a new day
  if (!(now.day() == rtc.now().day()))
  {
    wateredToday = false;
  }

  now = rtc.now();

  // log time
  logfile.print(now.unixtime()); // seconds since 2000
  logfile.print(",");
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print(",");
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

  soilMoistureRaw = analogRead(soilMoisturePin) * (3.3 / 1024);
  delay(20);

  //Volumetric Water Content is a piecewise function of the voltage from the sensor
  if (soilMoistureRaw < 1.1)
  {
    soilMoisture = (10 * soilMoistureRaw) - 1;
  }
  else if (soilMoistureRaw < 1.3)
  {
    soilMoisture = (25 * soilMoistureRaw) - 17.5;
  }
  else if (soilMoistureRaw < 1.82)
  {
    soilMoisture = (48.08 * soilMoistureRaw) - 47.5;
  }
  else if (soilMoistureRaw < 2.2)
  {
    soilMoisture = (26.32 * soilMoistureRaw) - 7.89;
  }
  else
  {
    soilMoisture = (62.5 * soilMoistureRaw) - 87.5;
  }

  humidity = dht.readHumidity();
  delay(20);

  airTemp = dht.readTemperature(true);
  delay(20);

  heatIndex = dht.computeHeatIndex(airTemp, humidity);

  //This is a rough conversion that I tried to calibrate using a flashlight of a "known" brightness
  sunlight = pow(((((150 * 3.3) / (analogRead(sunlightPin) * (3.3 / 1024))) - 150) / 70000), -1.25);
  delay(20);

  //Log variables
  logfile.print(soilTemp);
  logfile.print(",");
  logfile.print(airTemp);
  logfile.print(",");
  logfile.print(soilMoisture);
  logfile.print(",");
  logfile.print(humidity);
  logfile.print(",");
  logfile.print(heatIndex);
  logfile.print(",");
  logfile.print(sunlight);
  logfile.print(",");
#if ECHO_TO_SERIAL
  Serial.print(soilTemp);
  Serial.print(",");
  Serial.print(airTemp);
  Serial.print(",");
  Serial.print(soilMoisture);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
  Serial.print(heatIndex);
  Serial.print(",");
  Serial.print(sunlight);
  Serial.print(",");
#endif

  if ((soilMoisture < wateringThreshold) && (now.hour() > 19) && (now.hour() < 22) && (wateredToday = false))
  {
    //water the garden
    digitalWrite(solenoidPin, HIGH);
    delay(wateringTime);
    digitalWrite(solenoidPin, LOW);

    //record that we're watering
    logfile.print("TRUE");
#if ECHO_TO_SERIAL
    Serial.print("TRUE");
#endif

    wateredToday = true;
  }
  else
  {
    logfile.print("FALSE");
#if ECHO_TO_SERIAL
    Serial.print("FALSE");
#endif
  }

  logfile.println();
#if ECHO_TO_SERIAL
  Serial.println();
#endif
  delay(50);

  //Write to SD card
  logfile.flush();
  delay(5000);
}
