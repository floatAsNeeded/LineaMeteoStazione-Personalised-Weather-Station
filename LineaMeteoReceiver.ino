////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///****************************************LINEAMETEO STAZIONE RECEIVER********************************************///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////LIBRARY DECLARATION////////////////////////////////////////
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include "FirebaseESP8266.h"    //https://github.com/mobizt/Firebase-ESP8266
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP_Mail_Client.h>   //https://github.com/mobizt/ESP-Mail-Client
#include <String.h>
#include <Wire.h>
#include "NTPClient.h"         //https://github.com/floatAsNeeded/LineaMeteoStazione-Personalised-Weather-Station/blob/main/NTPClient.zip
#include <WiFiUDP.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <Adafruit_BMP085.h>    //https://github.com/adafruit/Adafruit-BMP085-Library
#include <math.h>
Adafruit_BMP085 bmp;
WiFiServer server(80);
WiFiClient client;
WiFiManager wifiManager;
#include <BlynkSimpleEsp8266.h>  //https://github.com/blynkkk/blynk-library
//byte Countrestart;

////////////////////*********EMAIL AND OTA DETAILS************///////////////////////////////////
//OTAREMOTE//
#define ProductKey "" // Replace with your own APIkey
#define Version "1.0.1.6"
#define MakeFirmwareInfo(k, v) "&_FirmwareInfo&k=" k "&v=" v "&FirmwareInfo_&"
unsigned long TIMER = 0;
void update();

//OTA LOCAL//
byte WiFiReset = 0;
const char* host = "esp8266-webupdate";
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
String DEVICE2OTA;
unsigned long TIMEROTA;

//EMAIL//
float TEMPERATUREMAXALERT;
float TEMPERATUREMINALERT;
float TEMPERATUREMAXALERTSECOND;
float TEMPERATUREMINALERTSECOND;
unsigned int HUMIDITYMAXALERT;
unsigned int HUMIDITYMINALERT;
unsigned int HUMIDITYMAXALERTSECOND;
unsigned int HUMIDITYMINALERTSECOND;
float GUSTALERT;
unsigned int UVALERT;
float RAINALERT;
float RAININTENSITYALERT;
float HEATINDEXALERT;
String Language;
String EMAILACCOUNT;
String EmailONOFF;
String EmailAlertONOFF;
String DIRECTIONWIND;
boolean Alert1 = true;
boolean Alert2 = true;
boolean Alert3 = true;
boolean Alert4 = true;
boolean Alert5 = true;
boolean Alert6 = true;
boolean Alert7 = true;
boolean Alert8 = true;
boolean Alert9 = true;
boolean Alert10 = true;
boolean Alert11 = true;
boolean Alert12 = true;
boolean Alert13 = true;
boolean Alert14 = true;
boolean Alert15 = true;
String Alert3ONOFF;
String Alert4ONOFF;
String Alert5ONOFF;
String Alert6ONOFF;
String Alert7ONOFF;
String Alert8ONOFF;
String Alert9ONOFF;
String Alert10ONOFF;
boolean NewAlertLimit = false;
unsigned long checkalert = 0;
unsigned long CheckEmail = 600000;
byte REPORTHOUR;
#define SMTP_HOST "smtp.gmail.com"

/** The smtp port e.g.
   25  or esp_mail_smtp_port_25
   465 or esp_mail_smtp_port_465
   587 or esp_mail_smtp_port_587
*/
#define SMTP_PORT 25

/* The log in credentials */
String AUTHOR_EMAIL;
String AUTHORPASSWORD;

/* The SMTP Session object used for Email sending */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);


////////////////////*********FIREBASE DETAILS************///////////////////////////////////
#define FIREBASE_HOST ""                 // the project name address from firebase id
#define FIREBASE_AUTH ""            // the secret key generated from firebase
FirebaseData Weather;


//////////////////////*********NTP SERVER************//////////////////////////////////////
// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS "pool.ntp.org" // "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)
// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS , NTP_OFFSET, NTP_INTERVAL);


///////////////////////*********BLYNK DETAILS************//////////////////////////////////
String APIBLYNK;
String ServerBlynk;
String BLYNKONOFF;
int Port;

////////////////////*********THINGSPEAK DETAILS************////////////////////////////////
String myWriteAPIKey;
const char* serverThingSpeak = "api.thingspeak.com";


////////////////////*********WEATHERCLOUD************////////////////////////////////
String Weathercloud_ID;
String Weathercloud_KEY;
const int httpPort = 80;
const char* Weathercloud = "api.weathercloud.net"; //http://api.weathercloud.net


////////////////////*********WUNDERGROUND************////////////////////////////////
char serverWU [] = "rtupdate.wunderground.com";
char WEBPAGE [] = "GET /weatherstation/updateweatherstation.php?";
String ID;
String Key;

////////////////////*********PWS************////////////////////////////////
char serverPWS [] = "pwsupdate.pwsweather.com";
char WEBPAGEPWS [] = "GET /api/v1/submitwx?";
String IDPWS;
String APIPWS;

////////////////////*********LINEAMETEO************////////////////////////////////
String Latitude;
String Longitude;
String City;
String Altitude;
String StringLineaMeteo;

//DEWPOINT CALCULATION CONSTANTS//
#define c1 -8.78469475556
#define c2 1.61139411
#define c3 2.33854883889
#define c4 -0.14611605
#define c5 -0.012308094
#define c6 -0.0164248277778
#define c7 0.002211732
#define c8 0.00072546
#define c9 -0.000003582


//////////TEMPERATURE///////////
float OffsetTemp;
float temp;
float tempSHT2x;
float tempinside;
//TEMPERATURE EXTREMES//
float maxTemp;
float minTemp;


//////////HUMIDITY/////////////
int OffsetHum;
int humiditySHT2x;
int humidity;
int humidityinside;
//HUMIDITY EXTREMES//
int maxHumidity;
int minHumidity;

//////////PRESSURE/////////////
float pressurehpa;
int CALIBRATION;


////////////RAIN////////////////
float mmGoccia; // tipping bucket count in mm
float mmPioggia;
float mmPioggiaLast = 0;
float rainrate = 0; // real-time rainrate
float rainrateMax = 0; // daily rainrateMax
byte rainratedetect = 0;
int rainrateMaxIntensity = 0;
byte PluvioFlag = 0; // detect interrupt of rain
byte rainrateI2C = 0;
byte rainrateMaxI2C = 0;
byte rainrateMaxI2CIntensity = 0;

////////////WIND/////////////////
int WindDirectionMisol;
int WindDirectionDavis;
float GustMax;
int Offset; // adjust wind direction value
byte Rotations;
byte GustRotations;
unsigned int average = 3000; // sample time for wind speed
float constant; // formula calculation
float WindSpeed; // speed km/h
float GustNow = 0;
int VaneValue; // raw analog value from wind vane
int Direction; // translated 0 - 360 direction
int CalDirection; // converted value with offset applied
byte AnemometerType;

////////////DEWPOINT///////////////
float dewPoint; // variabile del punto di rugiada
float dewPointIN;
//DEW POINT EXTREMES//
//float dewPointmax;
//float dewPointmin;


////////////HEATINDEX///////////////
float heatIndex; // variabile dell'indice di calore
float heatIndexIN;
//float heatIndexMax;


////////////WINDCHILL///////////////
float windchill; // variabile del raffreddamento causato dal vento
//float Windchillmin;


/////////////LIGHT////////////////
float UVindex;
float Lux;
float SolarRadiation;
float CalibrationRadiation;
float CalibrationUV;
//int UVmax;

/////////////AIRQUALITY////////////////
int AIRQUALITY;

////TIMING AND OTHER VARIALABLES////
unsigned long uploadtime = 0;
unsigned long timeout;
bool res;
unsigned long previous = 0;
unsigned long previousTHINGSPEAK;
unsigned long previousWEATHERCLOUD;
unsigned long previousWUNDERGROUND;
unsigned long previousBLYNK;
unsigned long previousPWS;
unsigned long UPLOADPWS = 300000;
unsigned long UPLOADTHINGSPEAK;
unsigned long UPLOADWEATHERCLOUD;
unsigned long UPLOADWUNDERGROUND;
unsigned long UPLOADBLYNK;
byte CurrentDay;
boolean readvalues = false;
byte RESETDATA = 0;
byte DATASENT;
int TIMEZONE;
long TIMEZONEINSECONDS;
float BatteryVoltage;
String AutomaticBatteryManagement;
byte HOURS;
byte PREVIOUSHOURS;
byte PREVIOUSREPORTHOUR;

//////////////////////////////////////////////////////
//////////////FUNCTIONS DECLARATIONS//////////////////
/////////////////////////////////////////////////////
//**************************************************//
/////////READ FROM DATABASE AND WRITE DATA////////////
//**************************************************//



//WIND//
void readDavisAnemometer () {
  if (Firebase.getInt(Weather, "/Time/Communication/Rotations"))
  {
    Rotations = Weather.to<int>();
  }
  if (Firebase.getInt(Weather, "/Time/Communication/GustRotations"))
  {
    GustRotations = Weather.to<int>();
  }
  if (Firebase.getInt(Weather, "/Time/Communication/WindDirectionDavis"))
  {
    WindDirectionDavis = Weather.to<int>();
  }
  //FORMULA V=P(Rotations)(2.25/T)--->constant
  constant = 2.25 / (average / 1000);
  WindSpeed = (Rotations * constant) * 1.60934;
  GustNow = (GustRotations * constant) * 1.60934;

  VaneValue = WindDirectionDavis;
  Direction = map(VaneValue, 0, 4095, 0, 360);
  CalDirection = Direction;
}

//WIND//
void readAnemometer () {

  if (Firebase.getInt(Weather, "/Time/Communication/Rotations"))
  {
    Rotations = Weather.to<int>();
  }
  if (Firebase.getInt(Weather, "/Time/Communication/GustRotations"))
  {
    GustRotations = Weather.to<int>();
  }

  if (Firebase.getInt(Weather, "/Time/Communication/WindDirectionMisol"))
  {
    WindDirectionMisol = Weather.to<int>();
  }


  WindSpeed = (Rotations * 2448.0) / average;
  GustNow = (GustRotations * 2448.0) / average;


  VaneValue = WindDirectionMisol;

  if (VaneValue < 250)
  {
    CalDirection = 90;
  }
  if (VaneValue > 250 && VaneValue < 600)
  {
    CalDirection = 135;
  }
  if (VaneValue > 600 && VaneValue < 1100)
  {
    CalDirection = 180;
  }
  if (VaneValue > 1100 && VaneValue < 2000)
  {
    CalDirection = 45;
  }
  if (VaneValue > 2000 && VaneValue < 2500 )
  {
    CalDirection = 225;
  }
  if (VaneValue > 2500 && VaneValue < 3100)
  {
    CalDirection = 0;
  }
  if (VaneValue > 3100 && VaneValue < 3650)
  {
    CalDirection = 315;
  }
  if (VaneValue > 3650 && VaneValue < 4095)
  {
    CalDirection = 270;
  }
}

void writeWind()
{
  Firebase.setFloat(Weather, "/Wind/WindSpeed", WindSpeed);
  Firebase.setInt(Weather, "/Wind/WindDirection", CalDirection);
  Firebase.setFloat(Weather, "/Wind/Gust", GustNow);
}

//RAIN//
void readRain () {
  if (Firebase.getInt(Weather, "/Time/Communication/PluvioFlag"))
  {
    PluvioFlag = Weather.to<int>();
  }
  if (Firebase.getInt(Weather, "/Time/Communication/rainrateI2C"))
  {
    rainrateI2C = Weather.to<int>();
  }
  if (Firebase.getInt(Weather, "/Time/Communication/rainrateMaxI2C"))
  {
    rainrateMaxI2C = Weather.to<int>();
  }
  if (Firebase.getInt(Weather, "/Time/Communication/rainrateMaxI2CIntensity"))
  {
    rainrateMaxI2CIntensity = Weather.to<int>();
  }

  mmPioggiaLast = (mmPioggiaLast + mmGoccia) * PluvioFlag;
  mmPioggia = mmPioggiaLast + mmPioggia;
  rainrate = (rainrateI2C / 0.3) * mmGoccia;
  rainrateMaxIntensity = rainrateMaxI2CIntensity * 10;

  if (rainrateMaxI2C > (rainrateMax / mmGoccia) * 0.3)
  {
    rainrateMax = (rainrateMaxI2C / 0.3) * mmGoccia;
    rainratedetect = 1;
  }
  if (rainrateMaxIntensity > (rainrateMax / mmGoccia) * 0.3)
  {
    rainrateMax = (rainrateMaxIntensity / 0.3) * mmGoccia;
    rainratedetect = 1;
  }
  mmPioggiaLast = 0;
}

void writeRain()
{
  if (PluvioFlag > 0)
  {
    Firebase.setFloat(Weather, "/Rain/Rain24H", mmPioggia);
  }
  Firebase.setFloat(Weather, "/Rain/RainRate", rainrate);
  if (rainratedetect == 1)
  {
    Firebase.setFloat(Weather, "/Rain/RainRateMax", rainrateMax);
    rainratedetect = 0;
  }
}

void readPressure()
{
  if (Firebase.getInt(Weather, "/Pressure/Calibration"))
  {
    CALIBRATION = Weather.to<int>();
  }
  float pressure = bmp.readPressure() + CALIBRATION;
  pressurehpa = pressure / 100;
}

void writePressure()
{
  Firebase.setFloat(Weather, "/Pressure/Pressure", pressurehpa);
}

void CheckTimezone()
{
  if (Firebase.getInt(Weather, "/Time/TIMEZONE"))
  {
    TIMEZONE = Weather.to<int>();
  }
}

void checkforUpdates()
{
  if (Firebase.getString(Weather, "/Connection/DEVICE2/Update"))
  {
    DEVICE2OTA = Weather.to<const char *>();
  }
}

void checkBatteryVoltage()
{
  if (Firebase.getFloat(Weather, "/Battery/CellVoltage"))
  {
    BatteryVoltage = Weather.to<float>();
  }
}

void readTemperatureHumidity()
{
  if (Firebase.getFloat(Weather, "/SHT3x/Temperature/Offset"))
  {
    OffsetTemp = Weather.to<float>();
  }

  if (Firebase.getInt(Weather, "/SHT3x/Humidity/Offset"))
  {
    OffsetHum = Weather.to<int>();
  }

  if (Firebase.getFloat(Weather, "/SHT3x/Temperature/Temperature"))
  {
    temp = Weather.to<float>() + OffsetTemp;
    if (OffsetTemp != 0)
    {
      Firebase.setFloat(Weather, "/SHT3x/Temperature/TemperatureAdjusted", temp);
    }
    else
    {
      Firebase.deleteNode(Weather, "/SHT3x/Temperature/TemperatureAdjusted");
    }
  }

  if (Firebase.getInt(Weather, "/SHT3x/Humidity/Humidity"))
  {
    humidity = Weather.to<int>() + OffsetHum;
    if (OffsetHum != 0)
    {
      Firebase.setInt(Weather, "/SHT3x/Humidity/HumidityAdjusted", humidity);
    }
    else
    {
      Firebase.deleteNode(Weather, "/SHT3x/Humidity/HumidityAdjusted");
    }
  }

  if (Firebase.getFloat(Weather, "/SHT2x/Temperature/Offset"))
  {
    OffsetTemp = Weather.to<float>();
  }

  if (Firebase.getInt(Weather, "/SHT2x/Humidity/Offset"))
  {
    OffsetHum = Weather.to<int>();
  }

  if (Firebase.getFloat(Weather, "/SHT2x/Temperature/Temperature"))
  {
    tempSHT2x = Weather.to<float>() + OffsetTemp;
    if (OffsetTemp != 0)
    {
      Firebase.setFloat(Weather, "/SHT2x/Temperature/TemperatureAdjusted", tempSHT2x);
    }
    else
    {
      Firebase.deleteNode(Weather, "/SHT2x/Temperature/TemperatureAdjusted");
    }
  }

  if (Firebase.getInt(Weather, "/SHT2x/Humidity/Humidity"))
  {
    humiditySHT2x = Weather.to<int>() + OffsetHum;
    if (OffsetHum != 0)
    {
      Firebase.setInt(Weather, "/SHT2x/Humidity/HumidityAdjusted", humiditySHT2x);
    }
    else
    {
      Firebase.deleteNode(Weather, "/SHT2x/Humidity/HumidityAdjusted");
    }
  }
}

void readInsideValues()
{
  if (Firebase.getFloat(Weather, "/Inside/Temperature"))
  {
    tempinside = Weather.to<float>();
  }

  if (Firebase.getInt(Weather, "/Inside/Humidity"))
  {
    humidityinside = Weather.to<int>();
  }
}

void readLight()
{
  if (Firebase.getFloat(Weather, "/Light/CalibrationUV"))
  {
    CalibrationUV = Weather.to<float>();
  }
  if (Firebase.getFloat(Weather, "/Light/CalibrationRadiation"))
  {
    CalibrationRadiation = Weather.to<float>();
  }


  if (Firebase.getFloat(Weather, "/Light/UVindex"))
  {
    UVindex = Weather.to<float>() * CalibrationUV;
    if (CalibrationUV != 1)
    {
      Firebase.setFloat(Weather, "/Light/UVindexAdjusted", UVindex);
    }
    else
    {
      Firebase.deleteNode(Weather, "/Light/UVindexAdjusted");
    }
  }

  if (Firebase.getFloat(Weather, "/Light/Lux"))
  {
    Lux = Weather.to<float>();
  }
}

void readAirQuality()
{
  if (Firebase.getInt(Weather, "/Inside/AirQuality"))
  {
    AIRQUALITY = Weather.to<int>();
  }
}

void readEmailLanguage()
{
  if (Firebase.getString(Weather, "/Services/EmailAlert/Language"))
  {
    Language = Weather.to<const char *>();
  }
  if (Language == "it")
  {
    if (CalDirection < 22)
      DIRECTIONWIND = "N";
    else if (CalDirection < 67)
      DIRECTIONWIND = "NE";
    else if (CalDirection < 112)
      DIRECTIONWIND = "E";
    else if (CalDirection < 157)
      DIRECTIONWIND = "SE";
    else if (CalDirection < 212)
      DIRECTIONWIND = "S";
    else if (CalDirection < 247)
      DIRECTIONWIND = "SO";
    else if (CalDirection < 292)
      DIRECTIONWIND = "O";
    else if (CalDirection < 337)
      DIRECTIONWIND = "NO";
    else
      DIRECTIONWIND = "N";
  }
  else
  {
    if (CalDirection < 22)
      DIRECTIONWIND = "N";
    else if (CalDirection < 67)
      DIRECTIONWIND = "NE";
    else if (CalDirection < 112)
      DIRECTIONWIND = "E";
    else if (CalDirection < 157)
      DIRECTIONWIND = "SE";
    else if (CalDirection < 212)
      DIRECTIONWIND = "S";
    else if (CalDirection < 247)
      DIRECTIONWIND = "SW";
    else if (CalDirection < 292)
      DIRECTIONWIND = "W";
    else if (CalDirection < 337)
      DIRECTIONWIND = "NW";
    else
      DIRECTIONWIND = "N";
  }
}

void readData()
{
  if (millis() - previous >= uploadtime)
  {
    previous = millis();
    uploadtime = 20000;
    timeClient.update();
    if (Firebase.getInt(Weather, "/Time/Communication/DataSent"))
    {
      DATASENT = Weather.to<int>();
    }
    checkforUpdates();
  }
  if (DATASENT == 1)
  {
    Firebase.setInt(Weather, "/Time/Communication/DataSent", 0);
    DATASENT = 0;
    readvalues = true;
    if (AnemometerType == 1)
    {
      readDavisAnemometer ();
    }
    else if (AnemometerType == 2)
    {
      readAnemometer ();
    }
    readRain();
    readPressure();
    readTemperatureHumidity();
    readInsideValues();
    readLight();
    readAirQuality();
    checkBatteryVoltage();
    readEmailLanguage();
  }
}

void writeData()
{
  if (readvalues == true)
  {
    readvalues = false;

    dewPoint = (temp - (14.55 + 0.114 * temp) * (1 - (0.01 * humidity)) - pow(((2.5 + 0.007 * temp) * (1 - (0.01 * humidity))), 3) - (15.9 + 0.117 * temp) * pow((1 - (0.01 * humidity)), 14));  //equazione per il calcolo del dewpoint
    //INSIDE//
    dewPointIN = (tempinside - (14.55 + 0.114 * tempinside) * (1 - (0.01 * humidityinside)) - pow(((2.5 + 0.007 * tempinside) * (1 - (0.01 * humidityinside))), 3) - (15.9 + 0.117 * tempinside) * pow((1 - (0.01 * humidityinside)), 14));  //equazione per il calcolo del dewpoint

    if (temp < 11)
    {
      windchill = (13.12 + 0.6215 * temp) - (11.37 * pow(WindSpeed, 0.16)) + (0.3965 * temp * pow(WindSpeed, 0.16));//equazione per il calcolo del windchill
    }
    else
    {
      windchill = temp;
    }

    if (temp > 21)
    {
      heatIndex = c1 + (c2 * temp) + (c3 * humidity) + (c4 * temp * humidity) + (c5 * sq(temp)) + (c6 * sq(humidity)) + (c7 * sq(temp) * humidity) + (c8 * temp * sq(humidity)) + (c9 * sq(temp) * sq(humidity));
    }
    else if (temp <= 21)
    {
      heatIndex = temp;
    }

    //INSIDE//
    if (tempinside > 21)
    {
      heatIndexIN = c1 + (c2 * tempinside) + (c3 * humidityinside) + (c4 * tempinside * humidityinside) + (c5 * sq(tempinside)) + (c6 * sq(humidityinside)) + (c7 * sq(tempinside) * humidityinside) + (c8 * tempinside * sq(humidityinside)) + (c9 * sq(tempinside) * sq(humidityinside));
    }
    else if (tempinside <= 21)
    {
      heatIndexIN = tempinside;
    }

    SolarRadiation = (Lux / 120) * CalibrationRadiation;
    if (SolarRadiation < 1)
    {
      SolarRadiation = 0;
    }
    Firebase.setFloat(Weather, "/Light/SolarRadiation", SolarRadiation);
    writePressure();
    writeRain();
    writeWind();
    maxminvalues();
    EMAILNOTIFICATION();
    
    if (millis() - previousWEATHERCLOUD >= UPLOADWEATHERCLOUD)
    {
      previousWEATHERCLOUD = millis();
      push_to_weathercloud();
      // pws();
    }
    if (millis() - previousWUNDERGROUND >= UPLOADWUNDERGROUND)
    {
      previousWUNDERGROUND = millis();
      wunderground();
    }
    if (BLYNKONOFF == "ON")
    {
      if (millis() - previousBLYNK >= UPLOADBLYNK)
      {
        previousBLYNK = millis();
        BlynkFunction();
      }
    }
    if (millis() - previousPWS >= UPLOADPWS)
    {
      previousPWS = millis();
      pws();
    }
    THINGSPEAK();
  }
}


////////////////DAYLY EXTREMES/////////////////////////
void maxminvalues()
{
  if (humidity > maxHumidity)
  {
    maxHumidity = humidity;
    Firebase.setInt(Weather, "/SHT3x/Humidity/HumidityMax", maxHumidity);
  }

  if (humidity < minHumidity)
  {
    minHumidity = humidity;
    Firebase.setInt(Weather, "/SHT3x/Humidity/HumidityMin", minHumidity);
  }

  if (Firebase.getInt(Weather, "/Wind/Offset"))
  {
    Offset = Weather.to<int>();
  }

  if (temp > maxTemp)
  {
    maxTemp = temp;
    Firebase.setFloat(Weather, "/SHT3x/Temperature/TemperatureMax", maxTemp);
  }
  if (temp < minTemp)
  {
    minTemp = temp;
    Firebase.setFloat(Weather, "/SHT3x/Temperature/TemperatureMin", minTemp);
  }
  CalDirection = CalDirection + Offset;

  if (CalDirection > 360) {
    CalDirection = CalDirection - 360;
  }
  if (CalDirection < 0) {
    CalDirection = CalDirection + 360;
  }

  if (Offset != 0)
  {
    Firebase.setFloat(Weather, "/Wind/WindDirectionAdjusted", CalDirection);
  }
  else
  {
    Firebase.deleteNode(Weather, "/Wind/WindDirectionAdjusted");
  }

  if (GustNow > GustMax)
  {
    GustMax = GustNow;
    Firebase.setFloat(Weather, "/Wind/GustMax", GustMax);
  }
}


///////////////////GET TIME//////////////////////////
void gettime()
{
  TIMEZONEINSECONDS = TIMEZONE * 3600;
  //timeClient.update();
  timeClient.setTimeOffset(TIMEZONEINSECONDS);
  timeClient.getFullFormattedTime();
  HOURS = timeClient.getHours();
  if (CurrentDay != timeClient.getDate() && timeClient.getYear() != 1970)
  {
    CurrentDay = timeClient.getDate();
    Alert2 = true;
    Alert11 = true;
    Alert13 = true;
    Alert14 = true;
    Alert15 = true;
    maxHumidity = humidity;
    minHumidity = humidity;
    maxTemp = temp;
    minTemp = temp;
    mmPioggia = 0;
    GustMax = 0;
    rainrateMax = 0;
    Firebase.setInt(Weather, "/Time/CurrentDay", CurrentDay);
    Firebase.setInt(Weather, "/SHT3x/Humidity/HumidityMax", maxHumidity);
    Firebase.setInt(Weather, "/SHT3x/Humidity/HumidityMin", minHumidity);
    Firebase.setFloat(Weather, "/SHT3x/Temperature/TemperatureMax", maxTemp);
    Firebase.setFloat(Weather, "/SHT3x/Temperature/TemperatureMin", minTemp);
    Firebase.setFloat(Weather, "/Rain/Rain24H", mmPioggia);
    Firebase.setFloat(Weather, "/Rain/RainRateMax", rainrateMax);
    Firebase.setFloat(Weather, "/Wind/GustMax", GustMax);
  }
}


///////////////////BATTERY MANAGEMENT//////////////////////////
void BATTERYMANAGEMENT()
{
  if (AutomaticBatteryManagement == "ON")
  {
    if (Firebase.getString(Weather, "/Battery/AutomaticBatteryManagement"))
    {
      AutomaticBatteryManagement = Weather.to<const char *>();
    }
    if (BatteryVoltage >= 4.1)
    {
      Firebase.setInt(Weather, "/Time/SampleTime", 60);
    }
    if (BatteryVoltage < 4.1 && BatteryVoltage >= 4.0)
    {
      Firebase.setInt(Weather, "/Time/SampleTime", 90);
    }
    if (BatteryVoltage < 4.0 && BatteryVoltage >= 3.9)
    {
      Firebase.setInt(Weather, "/Time/SampleTime", 120);
    }
    if (BatteryVoltage < 3.9 && BatteryVoltage >= 3.7)
    {
      Firebase.setInt(Weather, "/Time/SampleTime", 180);
    }
    if (BatteryVoltage < 3.7 && BatteryVoltage >= 3.6)
    {
      Firebase.setInt(Weather, "/Time/SampleTime", 240);
    }
    if (BatteryVoltage < 3.6 && BatteryVoltage >= 3.5)
    {
      Firebase.setInt(Weather, "/Time/SampleTime", 480);
    }
    if (BatteryVoltage < 3.5)
    {
      Firebase.setInt(Weather, "/Time/SampleTime", 1800);
    }
  }
  else
  {
    if (Firebase.getString(Weather, "/Battery/AutomaticBatteryManagement"))
    {
      AutomaticBatteryManagement = Weather.to<const char *>();
    }
  }
  if (BatteryVoltage >= 4.1)
  {
    Firebase.setInt(Weather, "/Time/SampleTime", 60);
  }
  if (BatteryVoltage < 3.5)
  {
    Firebase.setInt(Weather, "/Time/SampleTime", 1800);
  }
}


//////////////////////////////////EMAIL/////////////////////////////////////

void Email()
{
  if (millis() - checkalert >= CheckEmail)
  {
    ESP.wdtFeed(); // feeds the dog
    delay(0);
    checkalert = millis();
    BATTERYMANAGEMENT();
    if (EmailONOFF == "ON")
    {
      if (Firebase.getInt(Weather, "/Services/EmailAlert/ReportHour"))
      {
        REPORTHOUR = Weather.to<int>();
      }
      if (Firebase.getString(Weather, "/Services/EmailAlert/EmailReport"))
      {
        EmailONOFF = Weather.to<const char *>();
      }
    }
    else
    {
      if (Firebase.getString(Weather, "/Services/EmailAlert/EmailReport"))
      {
        EmailONOFF = Weather.to<const char *>();
      }
    }

    if (EmailAlertONOFF == "ON")
    {
      if (Firebase.getString(Weather, "/Services/EmailAlert/EmailAlert"))
      {
        EmailAlertONOFF = Weather.to<const char *>();
      }
      if (Firebase.getString(Weather, "/Services/EmailAlert/SHT3x/Temperature/Enable"))
      {
        Alert3ONOFF = Weather.to<const char *>();
      }
      if (Firebase.getString(Weather, "/Services/EmailAlert/SHT2x/Temperature/Enable"))
      {
        Alert4ONOFF = Weather.to<const char *>();
      }

      if (Firebase.getString(Weather, "/Services/EmailAlert/SHT3x/Humidity/Enable"))
      {
        Alert5ONOFF = Weather.to<const char *>();
      }

      if (Firebase.getString(Weather, "/Services/EmailAlert/SHT2x/Humidity/Enable"))
      {
        Alert6ONOFF = Weather.to<const char *>();
      }

      if (Firebase.getString(Weather, "/Services/EmailAlert/Gust/Enable"))
      {
        Alert7ONOFF = Weather.to<const char *>();
      }

      if (Firebase.getString(Weather, "/Services/EmailAlert/Rain/Enable"))
      {
        Alert8ONOFF = Weather.to<const char *>();
      }
      if (Firebase.getString(Weather, "/Services/EmailAlert/UV/Enable"))
      {
        Alert9ONOFF = Weather.to<const char *>();
      }
      if (Firebase.getString(Weather, "/Services/EmailAlert/HeatIndex/Enable"))
      {
        Alert10ONOFF = Weather.to<const char *>();
      }

      if (Alert3ONOFF == "ON")
      {
        if (Firebase.getFloat(Weather, "/Services/EmailAlert/SHT3x/Temperature/TemperatureMax"))
        {
          TEMPERATUREMAXALERT = Weather.to<float>();
        }
        if (Firebase.getFloat(Weather, "/Services/EmailAlert/SHT3x/Temperature/TemperatureMin"))
        {
          TEMPERATUREMINALERT = Weather.to<float>();
        }
      }

      if (Alert4ONOFF == "ON")
      {
        if (Firebase.getFloat(Weather, "/Services/EmailAlert/SHT2x/Temperature/TemperatureMax"))
        {
          TEMPERATUREMAXALERTSECOND = Weather.to<float>();
        }
        if (Firebase.getFloat(Weather, "/Services/EmailAlert/SHT2x/Temperature/TemperatureMin"))
        {
          TEMPERATUREMINALERTSECOND = Weather.to<float>();
        }
      }

      if (Alert5ONOFF == "ON")
      {
        if (Firebase.getInt(Weather, "/Services/EmailAlert/SHT3x/Humidity/HumidityMax"))
        {
          HUMIDITYMAXALERT = Weather.to<int>();
        }
        if (Firebase.getInt(Weather, "/Services/EmailAlert/SHT3x/Humidity/HumidityMin"))
        {
          HUMIDITYMINALERT = Weather.to<int>();
        }
      }

      if (Alert6ONOFF == "ON")
      {
        if (Firebase.getInt(Weather, "/Services/EmailAlert/SHT2x/Humidity/HumidityMax"))
        {
          HUMIDITYMAXALERTSECOND = Weather.to<int>();
        }
        if (Firebase.getInt(Weather, "/Services/EmailAlert/SHT2x/Humidity/HumidityMin"))
        {
          HUMIDITYMINALERTSECOND = Weather.to<int>();
        }
      }

      if (Alert7ONOFF == "ON")
      {
        if (Firebase.getFloat(Weather, "/Services/EmailAlert/Gust/Gust"))
        {
          GUSTALERT = Weather.to<float>();
        }

      }

      if (Alert8ONOFF == "ON")
      {
        if (Firebase.getFloat(Weather, "/Services/EmailAlert/Rain/Rain"))
        {
          RAINALERT = Weather.to<float>();
        }

        if (Firebase.getFloat(Weather, "/Services/EmailAlert/Rain/RainIntensity"))
        {
          RAININTENSITYALERT = Weather.to<float>();
        }
      }

      if (Alert9ONOFF == "ON")
      {
        if (Firebase.getInt(Weather, "/Services/EmailAlert/UV/UV"))
        {
          UVALERT = Weather.to<int>();
        }
      }

      if (Alert10ONOFF == "ON")
      {
        if (Firebase.getFloat(Weather, "/Services/EmailAlert/HeatIndex/HeatIndex"))
        {
          HEATINDEXALERT = Weather.to<float>();
        }
      }
      NewAlertLimit = true;
    }
    else
    {
      if (Firebase.getString(Weather, "/Services/EmailAlert/EmailAlert"))
      {
        EmailAlertONOFF = Weather.to<const char *>();
      }
    }
    if (HOURS != PREVIOUSREPORTHOUR)
    {
      PREVIOUSREPORTHOUR = HOURS;
      Alert1 = true;
      Alert3 = true;
      Alert4 = true;
      Alert4 = true;
      Alert5 = true;
      Alert6 = true;
      Alert7 = true;
      Alert8 = true;
      Alert9 = true;
      Alert10 = true;
      Alert12 = true;
    }
  }
}

void EMAILNOTIFICATION()
{
  ESP.wdtFeed(); // feeds the dog
  delay(0);
  if (HOURS != PREVIOUSHOURS && BatteryVoltage <= 3.45 && BatteryVoltage != 0)
  {
    PREVIOUSHOURS = HOURS;
    if (Language == "en")
    {
      /* Declare the session config data */
      ESP_Mail_Session session;

      /* Set the session config */
      session.server.host_name = SMTP_HOST;
      session.server.port = SMTP_PORT;
      session.login.email = AUTHOR_EMAIL.c_str();
      session.login.password = AUTHORPASSWORD.c_str();
      session.login.user_domain = "mydomain.net";

      /* Declare the message class */
      SMTP_Message message;

      /* Set the message headers */
      message.sender.name = "LineaMeteoStazione";
      message.sender.email = AUTHOR_EMAIL.c_str();
      message.subject = "BATTERY ALERT from your Weather Station!";
      message.addRecipient("Admin", EMAILACCOUNT.c_str());
      message.html.content = "<b><span style=\"color:#ff0000;\">Battery Level is lower than 3.45V, ALWAYS USE PROTECTED BATTERIES!</span></b><br><br>The Weather Station is already entered in BatterySaving Mode with 30 minutes sample time period.<br><br>Please check that solar panel or battery are currently working and installed correctly or that the solar panel installed has the correct wattage depending on your location.<br> Please check the instruction Manual and other information <a href=https://github.com/floatAsNeeded/LineaMeteoStazione-Personalised-Weather-Station> HERE</a> for more information.";

      /** The html text message character set e.g.
         us-ascii
         utf-8
         utf-7
         The default value is utf-8
      */
      message.html.charSet = "us-ascii";

      /** The content transfer encoding e.g.
         enc_7bit or "7bit" (not encoded)
         enc_qp or "quoted-printable" (encoded)
         enc_base64 or "base64" (encoded)
         enc_binary or "binary" (not encoded)
         enc_8bit or "8bit" (not encoded)
         The default value is "7bit"
      */
      message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

      /** The message priority
         esp_mail_smtp_priority_high or 1
         esp_mail_smtp_priority_normal or 3
         esp_mail_smtp_priority_low or 5
         The default value is esp_mail_smtp_priority_low
      */
      message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;

      /** The Delivery Status Notifications e.g.
         esp_mail_smtp_notify_never
         esp_mail_smtp_notify_success
         esp_mail_smtp_notify_failure
         esp_mail_smtp_notify_delay
         The default value is esp_mail_smtp_notify_never
      */
      message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

      /* Set the custom message header */
      message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

      /* Connect to server with the session config */
      if (!smtp.connect(&session))
        return;

      /* Start sending Email and close the session */
      if (!MailClient.sendMail(&smtp, &message))
        Serial.println("Error sending Email, " + smtp.errorReason());

    }
    else
    {
      /* Declare the session config data */
      ESP_Mail_Session session;

      /* Set the session config */
      session.server.host_name = SMTP_HOST;
      session.server.port = SMTP_PORT;
      session.login.email = AUTHOR_EMAIL.c_str();
      session.login.password = AUTHORPASSWORD.c_str();
      session.login.user_domain = "mydomain.net";

      /* Declare the message class */
      SMTP_Message message;

      /* Set the message headers */
      message.sender.name = "LineaMeteoStazione";
      message.sender.email = AUTHOR_EMAIL.c_str();
      message.subject = "AVVISO DI BATTERIA dalla tua LineaMeteoStazione Stazione Meteo!";
      message.addRecipient("Admin", EMAILACCOUNT.c_str());
      message.html.content = "<b><span style=\"color:#ff0000;\">Il livello della batteria è al momento inferiore a 3.4V. RICORDATI DI USARE SEMPRE BATTERIE PROTETTE!</span></b><br><br>La Stazione Meteo è già entrata in modalità di risparmio energetico con un periodo di campionamento di 30 minuti.<br><br>Si prega di verificare che il pannello solare o la batteria funzionino e siano installati correttamente o che il pannello solare installato abbia il wattaggio corretto a seconda della posizione geografica.<br>Per ulteriori informazioni consultare il manuale e altre informazioni <a href=https://github.com/floatAsNeeded/LineaMeteoStazione-Personalised-Weather-Station> QUI</a>";
      /** The html text message character set e.g.
         us-ascii
         utf-8
         utf-7
         The default value is utf-8
      */
      message.html.charSet = "us-ascii";

      /** The content transfer encoding e.g.
         enc_7bit or "7bit" (not encoded)
         enc_qp or "quoted-printable" (encoded)
         enc_base64 or "base64" (encoded)
         enc_binary or "binary" (not encoded)
         enc_8bit or "8bit" (not encoded)
         The default value is "7bit"
      */
      message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

      /** The message priority
         esp_mail_smtp_priority_high or 1
         esp_mail_smtp_priority_normal or 3
         esp_mail_smtp_priority_low or 5
         The default value is esp_mail_smtp_priority_low
      */
      message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

      /** The Delivery Status Notifications e.g.
         esp_mail_smtp_notify_never
         esp_mail_smtp_notify_success
         esp_mail_smtp_notify_failure
         esp_mail_smtp_notify_delay
         The default value is esp_mail_smtp_notify_never
      */
      message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

      /* Set the custom message header */
      message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

      /* Connect to server with the session config */
      if (!smtp.connect(&session))
        return;

      /* Start sending Email and close the session */
      if (!MailClient.sendMail(&smtp, &message))
        Serial.println("Error sending Email, " + smtp.errorReason());

      /* Set the message headers */
    }
  }

  if (BatteryVoltage >= 4.1 && Alert2 == true)
  {
    Alert2 = false;
    if (Language == "en")
    {
      /* Declare the session config data */
      ESP_Mail_Session session;

      /* Set the session config */
      session.server.host_name = SMTP_HOST;
      session.server.port = SMTP_PORT;
      session.login.email = AUTHOR_EMAIL.c_str();
      session.login.password = AUTHORPASSWORD.c_str();
      session.login.user_domain = "mydomain.net";

      /* Declare the message class */
      SMTP_Message message;

      /* Set the message headers */
      message.sender.name = "LineaMeteoStazione";
      message.sender.email = AUTHOR_EMAIL.c_str();
      message.subject = "BATTERY Notification from your LineaMeteoStazione Weather Station!";
      message.addRecipient("Admin", EMAILACCOUNT.c_str());
      message.html.content = "The battery is almost fully charged with 4.1V.<br> Although is good to have a good battery level for the Weather Station to be autonomous, it is not recommended to have for long periods a battery voltage of more than 4.1V. <br> For a better battery life and for a safe battery operation, use a solar panel that doesn't let the battery be for too long at more than 4.1V. <br> Sample time is now set to every minute.";
      /** The html text message character set e.g.
         us-ascii
         utf-8
         utf-7
         The default value is utf-8
      */
      message.html.charSet = "us-ascii";

      /** The content transfer encoding e.g.
         enc_7bit or "7bit" (not encoded)
         enc_qp or "quoted-printable" (encoded)
         enc_base64 or "base64" (encoded)
         enc_binary or "binary" (not encoded)
         enc_8bit or "8bit" (not encoded)
         The default value is "7bit"
      */
      message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

      /** The message priority
         esp_mail_smtp_priority_high or 1
         esp_mail_smtp_priority_normal or 3
         esp_mail_smtp_priority_low or 5
         The default value is esp_mail_smtp_priority_low
      */
      message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

      /** The Delivery Status Notifications e.g.
         esp_mail_smtp_notify_never
         esp_mail_smtp_notify_success
         esp_mail_smtp_notify_failure
         esp_mail_smtp_notify_delay
         The default value is esp_mail_smtp_notify_never
      */
      message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

      /* Set the custom message header */
      message.addHeader("Message-ID: <abcde.fghij@gmail.com>");


      /* Connect to server with the session config */
      if (!smtp.connect(&session))
        return;

      /* Start sending Email and close the session */
      if (!MailClient.sendMail(&smtp, &message))
        Serial.println("Error sending Email, " + smtp.errorReason());
    }
    else
    {
      /* Declare the session config data */
      ESP_Mail_Session session;

      /* Set the session config */
      session.server.host_name = SMTP_HOST;
      session.server.port = SMTP_PORT;
      session.login.email = AUTHOR_EMAIL.c_str();
      session.login.password = AUTHORPASSWORD.c_str();
      session.login.user_domain = "mydomain.net";

      /* Declare the message class */
      SMTP_Message message;

      /* Set the message headers */
      message.sender.name = "LineaMeteoStazione";
      message.sender.email = AUTHOR_EMAIL.c_str();
      message.subject = "Notifica BATTERIA dalla tua Stazione Meteo LineaMeteoStazione!";
      message.addRecipient("Admin", EMAILACCOUNT.c_str());
      message.html.content = "La batteria è quasi completamente carica con 4.1V. <br> Anche se è bene avere un buon livello di batteria per la stazione meteorologica per consentire un funzionamento autonomo, non è consigliabile avere per lunghi periodi la tensione della batteria superiore a 4.1V. <br> Per una migliore durata nel tempo della batteria e per un funzionamento sicuro della batteria, si consiglia di utilizzare un pannello solare che non permetta alla batteria di restare troppo a lungo con una tensione superiore a 4,1 V. <br> Il tempo di campionamento è ora impostato su 60 secondi.";
      /** The html text message character set e.g.
         us-ascii
         utf-8
         utf-7
         The default value is utf-8
      */
      message.html.charSet = "us-ascii";

      /** The content transfer encoding e.g.
         enc_7bit or "7bit" (not encoded)
         enc_qp or "quoted-printable" (encoded)
         enc_base64 or "base64" (encoded)
         enc_binary or "binary" (not encoded)
         enc_8bit or "8bit" (not encoded)
         The default value is "7bit"
      */
      message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

      /** The message priority
         esp_mail_smtp_priority_high or 1
         esp_mail_smtp_priority_normal or 3
         esp_mail_smtp_priority_low or 5
         The default value is esp_mail_smtp_priority_low
      */
      message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

      /** The Delivery Status Notifications e.g.
         esp_mail_smtp_notify_never
         esp_mail_smtp_notify_success
         esp_mail_smtp_notify_failure
         esp_mail_smtp_notify_delay
         The default value is esp_mail_smtp_notify_never
      */
      message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

      /* Set the custom message header */
      message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

      /* Connect to server with the session config */
      if (!smtp.connect(&session))
        return;

      /* Start sending Email and close the session */
      if (!MailClient.sendMail(&smtp, &message))
        Serial.println("Error sending Email, " + smtp.errorReason());
      /* Set the message headers */
    }
  }

  if (EmailONOFF == "ON")
  {
    if (REPORTHOUR == HOURS && Alert1 == true)
    {
      PREVIOUSREPORTHOUR = HOURS;
      Alert1 = false;
      if (Language == "en")
      {
        /* Declare the session config data */
        ESP_Mail_Session session;

        /* Set the session config */
        session.server.host_name = SMTP_HOST;
        session.server.port = SMTP_PORT;
        session.login.email = AUTHOR_EMAIL.c_str();
        session.login.password = AUTHORPASSWORD.c_str();
        session.login.user_domain = "mydomain.net";

        /* Declare the message class */
        SMTP_Message message;
        /* Set the message headers */
        message.sender.name = "LineaMeteoStazione";
        message.sender.email = AUTHOR_EMAIL.c_str();
        message.subject = "Today's Report from your Weather Station";
        message.addRecipient("Admin", EMAILACCOUNT.c_str());
        String html_msg =  "<br>Temperature is " + String(temp, 1) + "°C<br>Humidity is " + String(humidity) + "%<br>Pressure is " + String(pressurehpa, 1) + "hPa<br>Wind Speed is " + String(WindSpeed, 1) + "km/h from " + DIRECTIONWIND + "<br>Rain Today " + String(mmPioggia, 1) + "mm<br>Rain Intensity is " + String(rainrate, 1) + "mm/h<br>Current UV is " + String(UVindex, 1) + "<br>Solar Radiation is " + String(SolarRadiation, 1) + "W/m2<br><br><b>Daily Weather Observations</b><br><br>Highest " + String(maxTemp, 1) + "°C<br>Lowest " + String(minTemp, 1) + "°C<br>Gust " + String(GustNow, 1) + "km/h<br>Maximum Intensity of Rain " + String(rainrateMax, 1) + "mm/h<br><br><i>To stop this notification go to Services/EmailAlert/EmailReport and type OFF</i>";
        message.html.content = html_msg.c_str();

        /** The html text message character set e.g.
          us-ascii
          utf-8
          utf-7
          The default value is utf-8
        */
        message.html.charSet = "us-ascii";

        /** The content transfer encoding e.g.
           enc_7bit or "7bit" (not encoded)
           enc_qp or "quoted-printable" (encoded)
           enc_base64 or "base64" (encoded)
           enc_binary or "binary" (not encoded)
           enc_8bit or "8bit" (not encoded)
           The default value is "7bit"
        */
        message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

        /** The message priority
           esp_mail_smtp_priority_high or 1
           esp_mail_smtp_priority_normal or 3
           esp_mail_smtp_priority_low or 5
           The default value is esp_mail_smtp_priority_low
        */
        message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

        /** The Delivery Status Notifications e.g.
           esp_mail_smtp_notify_never
           esp_mail_smtp_notify_success
           esp_mail_smtp_notify_failure
           esp_mail_smtp_notify_delay
           The default value is esp_mail_smtp_notify_never
        */
        message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

        /* Set the custom message header */
        message.addHeader("Message-ID: <abcde.fghij@gmail.com>");


        /* Connect to server with the session config */
        if (!smtp.connect(&session))
          return;


        /* Start sending Email and close the session */
        if (!MailClient.sendMail(&smtp, &message))
          Serial.println("Error sending Email, " + smtp.errorReason());
      }
      else
      {
        /* Set the message headers */
        /**/

        /* Declare the session config data */
        ESP_Mail_Session session;

        /* Set the session config */
        session.server.host_name = SMTP_HOST;
        session.server.port = SMTP_PORT;
        session.login.email = AUTHOR_EMAIL.c_str();
        session.login.password = AUTHORPASSWORD.c_str();
        session.login.user_domain = "mydomain.net";

        /* Declare the message class */
        SMTP_Message message;
        /* Set the message headers */
        message.sender.name = "LineaMeteoStazione";
        message.sender.email = AUTHOR_EMAIL.c_str();
        message.subject = "Resoconto Giornaliero dalla tua Stazione Meteo";
        message.addRecipient("Admin", EMAILACCOUNT.c_str());
        String html_msg =  "<br>Temperatura attuale" + String(temp, 1) + "°C<br>Umidità relativa " + String(humidity) + "%<br>Pressione Atmosferica " + String(pressurehpa, 1) + "hPa<br>Velocità del vento " + String(WindSpeed, 1) + "km/h from " + DIRECTIONWIND + "<br>Precipitazioni oggi " + String(mmPioggia, 1) + "mm<br>Intensità della pioggia " + String(rainrate, 1) + "mm/h<br>Indice UV " + String(UVindex, 1) + "<br>Radiazione Solare " + String(SolarRadiation, 1) + "W/m2<br><br><b>Estremi Giornalieri</b><br><br>Temperatura Massima " + String(maxTemp, 1) + "°C<br>Temperatura Minima " + String(minTemp, 1) + "°C<br>Raffica di Vento " + String(GustNow, 1) + "km/h<br>Maximum Intensity of Rain " + String(rainrateMax, 1) + "mm/h<br><br><br><i>Se vuoi rimuovere queste notifiche vai su Services/EmailAlert/EmailReport e digita OFF</i>";
        message.html.content = html_msg.c_str();

        /** The html text message character set e.g.
          us-ascii
          utf-8
          utf-7
          The default value is utf-8
        */
        message.html.charSet = "us-ascii";

        /** The content transfer encoding e.g.
           enc_7bit or "7bit" (not encoded)
           enc_qp or "quoted-printable" (encoded)
           enc_base64 or "base64" (encoded)
           enc_binary or "binary" (not encoded)
           enc_8bit or "8bit" (not encoded)
           The default value is "7bit"
        */
        message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

        /** The message priority
           esp_mail_smtp_priority_high or 1
           esp_mail_smtp_priority_normal or 3
           esp_mail_smtp_priority_low or 5
           The default value is esp_mail_smtp_priority_low
        */
        message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;

        /** The Delivery Status Notifications e.g.
           esp_mail_smtp_notify_never
           esp_mail_smtp_notify_success
           esp_mail_smtp_notify_failure
           esp_mail_smtp_notify_delay
           The default value is esp_mail_smtp_notify_never
        */
        message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

        /* Set the custom message header */
        message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

        /* Connect to server with the session config */
        if (!smtp.connect(&session))
          return;

        /* Start sending Email and close the session */
        if (!MailClient.sendMail(&smtp, &message))
          Serial.println("Error sending Email, " + smtp.errorReason());
      }
    }
  }

  if (NewAlertLimit == true)
  {
    NewAlertLimit == false;
    if (Alert3ONOFF == "ON")
    {
      if (temp > TEMPERATUREMAXALERT && Alert3 == true)
      {
        Alert3 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Temperature Max Alert SHT3x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Temperature reached " + String(temp, 1) + "°C" + " more than the threshold set of " + String(TEMPERATUREMAXALERT, 1) + "°C" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/SHT3x/Temperature/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Temperatura Massima Allerta SHT3x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "La Temperatura ha raggiunto " + String(temp, 1) + "°C" + " più del limite impostato di " + String(TEMPERATUREMAXALERT, 1) + "°C" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/SHT3x/Temperature/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }



      if (temp < TEMPERATUREMINALERT && Alert4 == true)
      {
        Alert4 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Temperature Min Alert SHT3x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Temperature reached " + String(temp, 1) + "°C" + " less than the threshold set of " + String(TEMPERATUREMAXALERT, 1) + "°C" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/SHT3x/Temperature/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Temperatura Minima Allerta SHT3x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "La Temperatura ha raggiunto " + String(temp, 1) + "°C" + " meno del limite impostato di " + String(TEMPERATUREMAXALERT, 1) + "°C" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/SHT3x/Temperature/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }
    }

    if (Alert4ONOFF == "ON")
    {
      if (tempSHT2x < TEMPERATUREMINALERT && Alert5 == true)
      {
        Alert5 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Temperature Min Alert SHT2x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Temperature reached " + String(tempSHT2x, 1) + "°C" + " less than the threshold set of " + String(TEMPERATUREMINALERTSECOND, 1) + "°C" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/SHT2x/Temperature/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Temperatura Minima Allerta SHT2x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "La Temperatura ha raggiunto " + String(tempSHT2x, 1) + "°C" + " meno del limite impostato di " + String(TEMPERATUREMINALERTSECOND, 1) + "°C" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/SHT2x/Temperature/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }

      if (tempSHT2x > TEMPERATUREMAXALERTSECOND && Alert6 == true)
      {
        Alert6 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Temperature Max Alert SHT2x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Temperature reached " + String(tempSHT2x, 1) + "°C" + " more than the threshold set of " + String(TEMPERATUREMAXALERTSECOND, 1) + "°C" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/SHT2x/Temperature/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Temperatura Massima Allerta SHT2x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "La Temperatura ha raggiunto " + String(tempSHT2x, 1) + "°C" + " più del limite impostato di " + String(TEMPERATUREMAXALERTSECOND, 1) + "°C" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/SHT2x/Temperature/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }
    }

    if (Alert5ONOFF == "ON")
    {
      if (humidity > HUMIDITYMAXALERT && Alert7 == true)
      {
        Alert7 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Humidity Max Alert SHT3x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Humidity reached " + String(humidity) + "%" + " more than the threshold set of " + String(HUMIDITYMAXALERT) + "%" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/SHT3x/Humidity/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Umidità Massima Allerta SHT3x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "L'Umidità ha raggiunto " + String(humidity) + "%" + " più del limite impostato di " + String(HUMIDITYMAXALERT) + "%" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/SHT3x/Humidity/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }

      if (humidity < HUMIDITYMINALERT && Alert8 == true)
      {
        Alert8 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Humidity Min Alert SHT3x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Humidity reached " + String(humidity) + "%" + " less than the threshold set of " + String(HUMIDITYMINALERT) + "%" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/SHT3x/Humidity/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Umidità Minima Allerta SHT3x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "L'Umidità ha raggiunto " + String(humidity) + "%" + " meno del limite impostato di " + String(HUMIDITYMINALERT) + "%" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/SHT3x/Humidity/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }
    }

    if (Alert6ONOFF == "ON")
    {
      if (humiditySHT2x > HUMIDITYMAXALERTSECOND && Alert9 == true)
      {
        Alert9 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Humidity Max Alert SHT2x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Humidity reached " + String(humiditySHT2x) + "%" + " more than the threshold set of " + String(HUMIDITYMAXALERTSECOND) + "%" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/SHT2x/Humidity/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Umidità Massima Allerta SHT2x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "L'Umidità ha raggiunto " + String(humiditySHT2x) + "%" + " più del limite impostato di " + String(HUMIDITYMAXALERTSECOND) + "%" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/SHT2x/Humidity/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }

      if (humiditySHT2x < HUMIDITYMINALERTSECOND && Alert10 == true)
      {
        Alert10 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Humidity Min Alert SHT2x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Humidity reached " + String(humiditySHT2x) + "%" + " less than the threshold set of " + String(HUMIDITYMINALERTSECOND) + "%" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/SHT2x/Humidity/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Umidità Minima Allerta SHT2x";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "L'Umidità ha raggiunto " + String(humiditySHT2x) + "%" + " meno del limite impostato di " + String(HUMIDITYMINALERTSECOND) + "%" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/SHT2x/Humidity/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }
    }

    if (Alert7ONOFF == "ON")
    {
      if (GustMax > GUSTALERT && Alert11 == true)
      {
        Alert11 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Wind Speed Alert, Gust Max";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "The Wind Speed reached " + String(GustMax, 1) + "km/h" + " more than the threshold set of " + String(GUSTALERT, 1) + "km/h" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/Gust/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Raffica Massima di vento allerta";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "La velocità del vento ha raggiunto " + String(GustMax, 1) + "km/h" + " più del limite impostato di " + String(GUSTALERT, 1) + "km/h" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/Gust/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }
    }

    if (Alert8ONOFF == "ON")
    {
      if (mmPioggia > RAINALERT && Alert13 == true)
      {
        Alert13 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Precipitation Alert";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Rain precipitation today reached " + String(mmPioggia, 1) + "mm" + " more than the threshold set of " + String(RAINALERT, 1) + "mm" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/Rain/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Precipitazioni Pioggia Allerta";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "La pioggia accumulata oggi è " + String(mmPioggia, 1) + "mm" + " più del limite impostato di " + String(RAINALERT, 1) + "mm" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/Rain/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }


      if (rainrateMax > RAININTENSITYALERT && Alert14 == true)
      {
        Alert14 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Precipitation Intensity Alert";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Rain Intensity today reached " + String(rainrateMax, 1) + "mm/h" + " more than the threshold set of " + String(RAININTENSITYALERT, 1) + "mm/h" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/Rain/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Allerta Intensità pioggia";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "L'intensità della pioggia ha raggiunto " + String(rainrateMax, 1) + "mm/h" + " più del limite impostato di " + String(RAININTENSITYALERT, 1) + "mm/h" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/Rain/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }
    }

    if (Alert9ONOFF == "ON")
    {
      if (UVindex > UVALERT && Alert12 == true)
      {
        Alert12 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "UV Index Alert";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "The UV index reached " + String(UVindex) + " more than the threshold set of " + String(UVALERT) + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/UV/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Indice UV allerta";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "L'indice UV ha raggiunto " + String(UVindex) + " più del limite impostato di " + String(UVindex) + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/UV/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }
    }

    if (Alert10ONOFF == "ON")
    {
      if (heatIndex > HEATINDEXALERT && Alert15 == true)
      {
        Alert15 = false;
        if (Language == "en")
        {
          /* Declare the session config data */
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Heat Index Alert";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "Heat Index today reached " + String(heatIndex, 1) + "°C" + " more than the threshold set of " + String(HEATINDEXALERT, 1) + "°C" + "<br><br><i>To stop all alert notification go to Services/EmailAlert/EmailAlert and type OFF or to stop just this notification go to Services/EmailAlert/HeatIndex/Enable and type OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
        else
        {
          ESP_Mail_Session session;

          /* Set the session config */
          session.server.host_name = SMTP_HOST;
          session.server.port = SMTP_PORT;
          session.login.email = AUTHOR_EMAIL.c_str();
          session.login.password = AUTHORPASSWORD.c_str();
          session.login.user_domain = "mydomain.net";

          /* Declare the message class */
          SMTP_Message message;

          /* Set the message headers */
          message.sender.name = "LineaMeteoStazione";
          message.sender.email = AUTHOR_EMAIL.c_str();
          message.subject = "Allerta Indice di Calore";
          message.addRecipient("Admin", EMAILACCOUNT.c_str());
          String html_msg = "L'Indice di Calore ha raggiunto " + String(heatIndex, 1) + "°C" + " più del limite impostato di " + String(HEATINDEXALERT, 1) + "°C" + "<br><br><br><i>Se vuoi rimuovere tutte queste notifiche vai su Services/EmailAlert/EmailAlert e digita OFF o solo questa notifica vai su Services/EmailAlert/HeatIndex/Enable e digita OFF</i>";
          message.html.content = html_msg.c_str();

          /** The html text message character set e.g.
            us-ascii
            utf-8
            utf-7
            The default value is utf-8
          */

          message.html.charSet = "us-ascii";

          /** The content transfer encoding e.g.
             enc_7bit or "7bit" (not encoded)
             enc_qp or "quoted-printable" (encoded)
             enc_base64 or "base64" (encoded)
             enc_binary or "binary" (not encoded)
             enc_8bit or "8bit" (not encoded)
             The default value is "7bit"
          */
          message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

          /** The message priority
             esp_mail_smtp_priority_high or 1
             esp_mail_smtp_priority_normal or 3
             esp_mail_smtp_priority_low or 5
             The default value is esp_mail_smtp_priority_low
          */
          message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

          /** The Delivery Status Notifications e.g.
             esp_mail_smtp_notify_never
             esp_mail_smtp_notify_success
             esp_mail_smtp_notify_failure
             esp_mail_smtp_notify_delay
             The default value is esp_mail_smtp_notify_never
          */
          message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

          /* Set the custom message header */
          message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

          /* Connect to server with the session config */
          if (!smtp.connect(&session))
            return;

          /* Start sending Email and close the session */
          if (!MailClient.sendMail(&smtp, &message))
            Serial.println("Error sending Email, " + smtp.errorReason());
          /* Set the message headers */
        }
      }
    }
  }
}
//*****************************************************//
//////////////WEBSERVER AND HOST FOR DATA////////////////
//*****************************************************//

void THINGSPEAK()
{
  if (millis() - previousTHINGSPEAK >= UPLOADTHINGSPEAK)
  {
    previousTHINGSPEAK = millis();
    /*url = "/update?api_key=" + myWriteAPIKey + "&field1=" + (String)(temp) + "&field2=" + (String)(humidity) + "&field3=" + (String)(tempSHT1x) + "&field4=" + (String)(humiditySHT1x) + "&field5=" + (String)(WindSpeed) + "&field6=" + (String)(GustNow);
      Serial.println(host + url);
      http.begin(host + url);
      if (http.GET() == HTTP_CODE_OK) Serial.println(http.getString());
      http.end();*/
    WiFiClient client;
    if (client.connect(serverThingSpeak, 80)) { // use ip 184.106.153.149 or api.thingspeak.com
      Serial.print("connected to ");
      Serial.println(client.remoteIP());

      String postStr = myWriteAPIKey;
      postStr += "&field1=";
      postStr += String(temp);
      postStr += "&field2=";
      postStr += String(humidity);
      postStr += "&field3=";
      postStr += String(tempSHT2x);
      postStr += "&field4=";
      postStr += String(humiditySHT2x);
      postStr += "&field5=";
      postStr += String(WindSpeed);
      postStr += "&field6=";
      postStr += String(GustNow);
      postStr += "&field7=";
      postStr += String(SolarRadiation);
      /*postStr += "&field8=";
        postStr += String(quality);
        postStr += "\r\n\r\n";*/

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + myWriteAPIKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
      delay(50);
    }
    client.stop();
  }
}

void pws()
{
  Serial.print("Connecting to ");
  Serial.println(serverPWS);
  WiFiClient client;
  if (client.connect(serverPWS, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    delay(100);
  } else {
    Serial.println("connection failed");
  }
  client.print(WEBPAGEPWS);
  client.print("ID=");
  client.print(IDPWS);
  client.print("&PASSWORD=");
  client.print(APIPWS);
  client.print("&dateutc=now&winddir=");
  client.print(CalDirection);
  client.print("&tempf=");
  client.print(temp * 1.8 + 32);
  client.print("&windspeedmph=");
  client.print(WindSpeed / 1.609);
  client.print("&windgustmph=");
  client.print(GustNow / 1.609);
  client.print("&dewptf=");
  client.print(dewPoint * 1.8 + 32);
  client.print("&humidity=");
  client.print(humidity);
  client.print("&baromin=");
  client.print(pressurehpa * 0.02953);
  client.print("&rainin=");
  client.print(rainrate / 25.4);
  client.print("&dailyrainin=");
  client.print(mmPioggia / 25.4);
  client.print("&softwaretype=LineaMeteoStazione&action=updateraw");
  client.print("/ HTTP/1.1\r\nHost: pwsupdate.pwsweather.com:80\r\nConnection: close\r\n\r\n");
  Serial.println(" ");
  delay(100);
}

void lineameteo()
{
  WiFiClient client = server.available();

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print(timeClient.getFullFormattedTime());
  client.print("&v0=");
  client.print(Latitude);
  client.print("&v0=");
  client.print(Longitude);
  client.print("&v0=");
  client.print(City);
  client.print("&v0=");
  client.print(Altitude);
  client.print("&v0=");
  client.print(temp, 1);
  client.print("&v0=");
  client.print(humidity);
  client.print("&v0=");
  client.print(pressurehpa, 1);
  client.print("&v0=");
  client.print(WindSpeed, 1);
  client.print("&v0=");
  client.print(DIRECTIONWIND);
  client.print("&v0=");
  client.print(mmPioggia, 1);
  client.print("&v0=");
  client.print(minTemp, 1);
  client.print("&v0=");
  client.print(maxTemp, 1);
  client.print("&v0=");
  client.print(GustMax, 1);
  client.print("&v0=");
  client.print(rainrate, 1);
  client.println("</html>");

}

void push_to_weathercloud()
{
  //Serial.println("Initializing data push to Weathercloud.");

  if (!client.connect(Weathercloud, httpPort)) {
    Serial.println("Connecting to Weatherloud failed.\n");
    return;
  }
  client.print("GET /set");
  client.print("/wid/"); client.print(Weathercloud_ID);
  client.print("/key/"); client.print(Weathercloud_KEY);

  client.print("/temp/"); client.print(temp * 10);
  client.print("/tempin/"); client.print(tempinside * 10);
  client.print("/humin/"); client.print(humidityinside);
  client.print("/chill/"); client.print(windchill * 10);
  client.print("/dew/"); client.print(dewPoint * 10);
  client.print("/dewin/"); client.print(dewPointIN * 10);
  client.print("/heat/"); client.print(heatIndex * 10);
  client.print("/heatin/"); client.print(heatIndexIN * 10);
  client.print("/hum/"); client.print(humidity);
  client.print("/wspd/"); client.print((WindSpeed * 10) / 3.6);
  client.print("/wspdhi/"); client.print((GustNow * 10) / 3.6);
  client.print("/wdir/"); client.print(CalDirection);
  client.print("/bar/"); client.print(pressurehpa * 10);
  client.print("/rain/"); client.print(mmPioggia * 10);
  client.print("/rainrate/"); client.print(rainrate * 10);
  client.print("/uvi/"); client.print(UVindex * 10);
  client.print("/solarrad/"); client.print(SolarRadiation * 10);
  client.print("/aqi/"); client.print(AIRQUALITY * 10);
  client.println("/ HTTP/1.1");
  client.println("Host: api.weathercloud.net");
  client.println("Connection: close");
  client.println();
  Serial.println("Data pushed to Weathercloud sucessfuly.\n");;
}

void wunderground()
{
  Serial.print("Connecting to ");
  Serial.println(serverWU);
  WiFiClient client;
  if (client.connect(serverWU, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    delay(100);
  } else {
    Serial.println("connection failed");
  }
  client.print(WEBPAGE);
  client.print("ID=");
  client.print(ID);
  client.print("&PASSWORD=");
  client.print(Key);
  client.print("&dateutc=now&winddir=");
  client.print(CalDirection);
  client.print("&tempf=");
  client.print(temp * 1.8 + 32);
  client.print("&windspeedmph=");
  client.print(WindSpeed / 1.609);
  client.print("&windgustmph=");
  client.print(GustNow / 1.609);
  client.print("&dewptf=");
  client.print(dewPoint * 1.8 + 32);
  client.print("&humidity=");
  client.print(humidity);
  client.print("&baromin=");
  client.print(pressurehpa * 0.02953);
  client.print("&rainin=");
  client.print(rainrate / 25.4);
  client.print("&dailyrainin=");
  client.print(mmPioggia / 25.4);
  client.print("&UV=");
  client.print(UVindex);
  client.print("&solarradiation=");
  client.print(SolarRadiation);
  client.print("&softwaretype=Linea-Meteo&action=updateraw&realtime=1&rtfreq=30");
  client.print("/ HTTP/1.1\r\nHost: rtupdate.wunderground.com:80\r\nConnection: close\r\n\r\n");
  //Serial.println(" ");
  delay(50);
}

void BlynkFunction()
{
  Blynk.connect();
  Blynk.virtualWrite(V0, temp );
  Blynk.virtualWrite(V1, humidity );
  Blynk.virtualWrite(V2, pressurehpa );
  Blynk.virtualWrite(V3, UVindex);
  Blynk.virtualWrite(V4, WindSpeed);
  Blynk.virtualWrite(V5, DIRECTIONWIND);
  Blynk.virtualWrite(V6, mmPioggia);
  Blynk.virtualWrite(V7, BatteryVoltage);
  Blynk.virtualWrite(V8, tempinside);
  Blynk.virtualWrite(V9, humidityinside);
  Blynk.virtualWrite(V10, Lux);
  Blynk.virtualWrite(V11, SolarRadiation);
  Blynk.virtualWrite(V12, AIRQUALITY);
  Blynk.virtualWrite(V13, tempSHT2x);
  Blynk.virtualWrite(V14, humiditySHT2x);
}

//**************************************************//
////////////////SETUP FUNCTIONS///////////////////////
//**************************************************//

void SetupData()
{
  Firebase.setString(Weather, "/Connection/DEVICE2/IPAddress", WiFi.localIP().toString());

  if (Firebase.getInt(Weather, "/Time/RESETDATA"))
  {
    RESETDATA = Weather.to<int>();
  }
  else
  {
    Serial.println(Weather.errorReason());
  }
  if (RESETDATA == 0)
  {
    Firebase.setInt(Weather, "/Connection/DEVICE1/ResetWiFi", 0);
    Firebase.setInt(Weather, "/Connection/DEVICE2/ResetWiFi", 0);
    Firebase.setInt(Weather, "/Connection/DEVICE3/ResetWiFi", 0);
    Firebase.setString(Weather, "/Connection/DEVICE1/Update", "disable");
    Firebase.setString(Weather, "/Connection/DEVICE2/Update", "disable");
    Firebase.setString(Weather, "/Connection/DEVICE3/Update", "disable");
    Firebase.setInt(Weather, "/Wind/Offset", 0);
    Firebase.setInt(Weather, "/Wind/Anemometer", 0);
    Firebase.setFloat(Weather, "/Rain/mmGoccia", 0.2);
    Firebase.setInt(Weather, "/Time/TIMEZONE", 1);
    Firebase.setInt(Weather, "/Time/RESETDATA", 1);
    Firebase.setInt(Weather, "/Time/SampleTime", 90);
    Firebase.setInt(Weather, "/Pressure/Calibration", 0);
    Firebase.setFloat(Weather, "/SHT3x/Temperature/Offset", 0.0);
    Firebase.setInt(Weather, "/SHT3x/Humidity/Offset", 0);
    Firebase.setInt(Weather, "/SHT3x/Humidity/HumidityMax", -100);
    Firebase.setInt(Weather, "/SHT3x/Humidity/HumidityMin", 100);
    Firebase.setFloat(Weather, "/SHT3x/Temperature/TemperatureMax", -100);
    Firebase.setFloat(Weather, "/SHT3x/Temperature/TemperatureMin", 100);
    Firebase.setFloat(Weather, "/SHT2x/Temperature/Offset", 0.0);
    Firebase.setInt(Weather, "/SHT2x/Humidity/Offset", 0);
    Firebase.setString(Weather, "/Services/PWS/ID", "YourID");
    Firebase.setString(Weather, "/Services/PWS/API", "YourKey");
    Firebase.setString(Weather, "/Services/Wunderground/ID", "YourID");
    Firebase.setString(Weather, "/Services/Wunderground/Key", "YourKey");
    Firebase.setInt(Weather, "/Services/Wunderground/TimeUpload", 120);
    Firebase.setString(Weather, "/Services/WeatherCloud/ID", "YourID");
    Firebase.setString(Weather, "/Services/WeatherCloud/Key", "YourKey");
    Firebase.setInt(Weather, "/Services/WeatherCloud/TimeUpload", 360);
    Firebase.setString(Weather, "/Services/ThingSpeak/myWriteAPIKey", "WriteKeyAPI");
    Firebase.setInt(Weather, "/Services/ThingSpeak/TimeUpload", 600);
    Firebase.setString(Weather, "/Services/OpenWeather/API", "API");
    Firebase.setString(Weather, "/Services/OpenWeather/Hemisphere", "north or south");
    Firebase.setString(Weather, "/Display/Language", "en");
    Firebase.setString(Weather, "/Display/Units/Temperature", "metric");
    Firebase.setString(Weather, "/Display/Units/Pressure", "metric");
    Firebase.setString(Weather, "/Display/Units/Wind", "metric");
    Firebase.setString(Weather, "/Display/Units/Rain", "metric");
    Firebase.setString(Weather, "/Display/FastRefresh", "NO");
    Firebase.setString(Weather, "/Services/OpenWeather/Latitude", "- for South + for North");
    Firebase.setString(Weather, "/Services/OpenWeather/Longitude", "- for West + for East");
    Firebase.setString(Weather, "/Services/LineaMeteo/Latitude", "- for South + for North");
    Firebase.setString(Weather, "/Services/LineaMeteo/Longitude", "- for West + for East");
    Firebase.setString(Weather, "/Services/LineaMeteo/City", "Perugia(PG)");
    Firebase.setString(Weather, "/Services/LineaMeteo/Altitude", "0m s.l.m.");
    Firebase.setString(Weather, "/Services/LineaMeteo/WebServerString", "disable");
    Firebase.setString(Weather, "/Services/EmailAlert/EmailAccountForSMPT", "account_forSMPT@gmail.com");
    Firebase.setString(Weather, "/Services/EmailAlert/Password", "PasswordofaccountforSMPT");
    Firebase.setString(Weather, "/Services/EmailAlert/EmailAccountForNotification", "account_to_send@gmail.com");
    Firebase.setString(Weather, "/Services/EmailAlert/EmailReport", "OFF");
    Firebase.setString(Weather, "/Services/EmailAlert/EmailAlert", "OFF");
    Firebase.setString(Weather, "/Services/EmailAlert/Language", "en");
    Firebase.setInt(Weather, "/Services/EmailAlert/ReportHour", 7);
    Firebase.setString(Weather, "/Services/EmailAlert/SHT3x/Temperature/Enable", "OFF");
    Firebase.setFloat(Weather, "/Services/EmailAlert/SHT3x/Temperature/TemperatureMax", 30);
    Firebase.setFloat(Weather, "/Services/EmailAlert/SHT3x/Temperature/TemperatureMin", 0);
    Firebase.setString(Weather, "/Services/EmailAlert/SHT3x/Humidity/Enable", "OFF");
    Firebase.setInt(Weather, "/Services/EmailAlert/SHT3x/Humidity/HumidityMax", 90);
    Firebase.setInt(Weather, "/Services/EmailAlert/SHT3x/Humidity/HumidityMin", 20);
    Firebase.setString(Weather, "/Services/EmailAlert/SHT2x/Temperature/Enable", "OFF");
    Firebase.setFloat(Weather, "/Services/EmailAlert/SHT2x/Temperature/TemperatureMax", 30);
    Firebase.setFloat(Weather, "/Services/EmailAlert/SHT2x/Temperature/TemperatureMin", 0);
    Firebase.setString(Weather, "/Services/EmailAlert/SHT2x/Humidity/Enable", "OFF");
    Firebase.setInt(Weather, "/Services/EmailAlert/SHT2x/Humidity/HumidityMax", 90);
    Firebase.setInt(Weather, "/Services/EmailAlert/SHT2x/Humidity/HumidityMin", 20);
    Firebase.setString(Weather, "/Services/EmailAlert/Gust/Enable", "OFF");
    Firebase.setFloat(Weather, "/Services/EmailAlert/Gust/Gust", 50);
    Firebase.setString(Weather, "/Services/EmailAlert/Rain/Enable", "OFF");
    Firebase.setFloat(Weather, "/Services/EmailAlert/Rain/Rain", 25);
    Firebase.setFloat(Weather, "/Services/EmailAlert/Rain/RainIntensity", 50);
    Firebase.setString(Weather, "/Services/EmailAlert/HeatIndex/Enable", "OFF");
    Firebase.setFloat(Weather, "/Services/EmailAlert/HeatIndex/HeatIndex", 30);
    Firebase.setString(Weather, "/Services/EmailAlert/UV/Enable", "OFF");
    Firebase.setInt(Weather, "/Services/EmailAlert/UV/UV", 6);
    Firebase.setString(Weather, "/Battery/AutomaticBatteryManagement", "ON");
    Firebase.setFloat(Weather, "/Light/CalibrationRadiation", 2.0);
    Firebase.setFloat(Weather, "/Light/CalibrationUV", 1.0);
    Firebase.setString(Weather, "/Services/Blynk/API", "Your API");
    Firebase.setInt(Weather, "/Services/Blynk/TimeUpload", 120);
    Firebase.setInt(Weather, "/Services/Blynk/Port", 80);
    Firebase.setString(Weather, "/Services/Blynk/Server", "blynk-cloud.com");
    Firebase.setString(Weather, "/Services/Blynk/Enable", "OFF");
  }
}

void getDataTime()
{
  if (Firebase.getInt(Weather, "/Wind/Anemometer"))
  {
    AnemometerType = Weather.to<int>();
  }

  if (Firebase.getFloat(Weather, "/Rain/mmGoccia"))
  {
    mmGoccia = Weather.to<float>();
  }
  if (Firebase.getFloat(Weather, "/Rain/Rain24H"))
  {
    mmPioggia = Weather.to<float>();
  }
  if (Firebase.getFloat(Weather, "/Rain/RainRateMax"))
  {
    rainrateMax = Weather.to<float>();
  }
  if (Firebase.getInt(Weather, "/Time/TIMEZONE"))
  {
    TIMEZONE = Weather.to<int>();
  }

  if (Firebase.getInt(Weather, "/Time/CurrentDay"))
  {
    CurrentDay = Weather.to<int>();
  }

  if (Firebase.getInt(Weather, "/SHT3x/Humidity/HumidityMax"))
  {
    maxHumidity = Weather.to<int>();
  }

  if (Firebase.getInt(Weather, "/SHT3x/Humidity/HumidityMin"))
  {
    minHumidity = Weather.to<int>();
  }

  if (Firebase.getFloat(Weather, "/SHT3x/Temperature/TemperatureMax"))
  {
    maxTemp = Weather.to<float>();
  }

  if (Firebase.getFloat(Weather, "/SHT3x/Temperature/TemperatureMin"))
  {
    minTemp = Weather.to<float>();
  }
  if (Firebase.getFloat(Weather, "/Wind/GustMax"))
  {
    GustMax = Weather.to<float>();
  }
  if (Firebase.getInt(Weather, "/Services/Wunderground/TimeUpload"))
  {
    UPLOADWUNDERGROUND = Weather.to<int>() * 1000;
  }
  if (Firebase.getString(Weather, "/Services/PWS/ID"))
  {
    IDPWS = Weather.to<const char *>();
  }
  if (Firebase.getString(Weather, "/Services/PWS/API"))
  {
    APIPWS = Weather.to<const char *>();
  }
  if (Firebase.getString(Weather, "/Services/Wunderground/ID"))
  {
    ID = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/Wunderground/Key"))
  {
    Key = Weather.to<const char *>();
  }
  if (Firebase.getInt(Weather, "/Services/WeatherCloud/TimeUpload"))
  {
    UPLOADWEATHERCLOUD = Weather.to<int>() * 1000;
  }
  if (Firebase.getString(Weather, "/Services/WeatherCloud/ID"))
  {
    Weathercloud_ID = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/WeatherCloud/Key"))
  {
    Weathercloud_KEY = Weather.to<const char *>();
  }

  if (Firebase.getInt(Weather, "/Services/ThingSpeak/TimeUpload"))
  {
    UPLOADTHINGSPEAK = Weather.to<int>() * 1000;
  }

  if (Firebase.getString(Weather, "/Services/ThingSpeak/myWriteAPIKey"))
  {
    myWriteAPIKey = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/LineaMeteo/Latitude"))
  {
    Latitude = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/LineaMeteo/Longitude"))
  {
    Longitude = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/LineaMeteo/City"))
  {
    City = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/LineaMeteo/Altitude"))
  {
    Altitude = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/LineaMeteo/WebServerString"))
  {
    StringLineaMeteo = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/EmailAlert/EmailAccountForSMPT"))
  {
    AUTHOR_EMAIL = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/EmailAlert/Password"))
  {
    AUTHORPASSWORD = Weather.to<const char *>();
  }

  if (Firebase.getString(Weather, "/Services/EmailAlert/EmailAccountForNotification"))
  {
    EMAILACCOUNT = Weather.to<const char *>();
  }
  if (Firebase.getString(Weather, "/Services/EmailAlert/Language"))
  {
    Language = Weather.to<const char *>();
  }
  if (Firebase.getString(Weather, "/Services/EmailAlert/EmailReport"))
  {
    EmailONOFF = Weather.to<const char *>();
  }
  if (Firebase.getString(Weather, "/Services/EmailAlert/EmailAlert"))
  {
    EmailAlertONOFF = Weather.to<const char *>();
  }
  if (Firebase.getString(Weather, "/Battery/AutomaticBatteryManagement"))
  {
    AutomaticBatteryManagement = Weather.to<const char *>();
  }
  if (Firebase.getString(Weather, "/Services/Blynk/API"))
  {
    APIBLYNK = Weather.to<const char *>();
  }
  if (Firebase.getString(Weather, "/Services/Blynk/Server"))
  {
    ServerBlynk = Weather.to<const char *>();
  }
  if (Firebase.getString(Weather, "/Services/Blynk/Enable"))
  {
    BLYNKONOFF = Weather.to<const char *>();
  }
  if (Firebase.getInt(Weather, "/Services/Blynk/TimeUpload"))
  {
    UPLOADBLYNK = Weather.to<int>() * 1000;
  }
  if (Firebase.getInt(Weather, "/Services/Blynk/Port"))
  {
    Port = Weather.to<int>();
  }
}


void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  //wifiManager.resetSettings();
  wifiManager.setConfigPortalTimeout(300);
  WiFi.begin();
  timeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    //Serial.print(".");
    if (millis() - timeout > 25000)
    {
      res = wifiManager.autoConnect("LineaMeteoStazioneR", "LaMeteo2005");

      if (!res) {
        //Serial.println("Failed to connect");
        //ESP.deepSleep(15 * 1000000, WAKE_RF_DEFAULT);
        ESP.restart();
      }
    }
  }
  timeClient.begin();   // Start the NTP UDP client
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   // connect to firebase
  Serial.println(Weather.errorReason());
  Firebase.reconnectWiFi(true);
  Firebase.setMaxRetry(Weather, 2);
  //Weather.setBSSLBufferSize(1024, 512);
  //Set the size of HTTP response buffers in the case where we want to work with large data.
  //Weather.setResponseSize(1024);
  //char auth[] = APIBLYNK.c_str();WiFi.SSID().c_str()
  if (Firebase.getInt(Weather, "/Connection/DEVICE2/ResetWiFi"))
  {
    WiFiReset = Weather.to<int>();
  }
  if (WiFiReset == 1)
  {
    WiFiReset = 0;
    Firebase.setInt(Weather, "/Connection/DEVICE2/ResetWiFi", 0);
    wifiManager.resetSettings();
    WiFi.mode(WIFI_STA);
    wifiManager.setConfigPortalTimeout(300);
    WiFi.begin();
    timeout = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      //Serial.print(".");
      if (millis() - timeout > 10000)
      {
        res = wifiManager.autoConnect("LineaMeteoStazioneR", "LaMeteo2005");

        if (!res) {
          ESP.restart();
        }
      }
    }
  }
  Firebase.deleteNode(Weather, "/Connection/DEVICE2/UpdateHere");
  Firebase.setString(Weather, "/Connection/DEVICE2/Version", Version);
  /*if (Firebase.getInt(Weather, "/Test/Restart"))
    {
    Countrestart = Weather.to<int>();
    }
    Countrestart++;
    Firebase.setInt(Weather, "/Test/Restart", Countrestart);*/
  SetupData();
  getDataTime();

  Wire.begin();
  bmp.begin();
  smtp.debug(1);
  if (BLYNKONOFF == "ON")
  {
    //Blynk.begin(APIBLYNK.c_str(), WiFi.SSID().c_str(), WiFi.psk().c_str(), ServerBlynk.c_str(), Port);
    Blynk.config(APIBLYNK.c_str(), ServerBlynk.c_str(), Port);
    Blynk.connect();
  }
  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  if (Firebase.getString(Weather, "/Connection/DEVICE2/Update"))
  {
    DEVICE2OTA = Weather.to<const char *>();
  }

  if (DEVICE2OTA == "enable")
  {
    Firebase.setString(Weather, "/Connection/DEVICE2/Update", "disable");
    delay(50);
    update();
  }
  if (DEVICE2OTA == "enable")
  {
    MDNS.begin(host);

    httpUpdater.setup(&httpServer);
    httpServer.begin();

    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
    Firebase.setString(Weather, "/Connection/DEVICE2/Update", "disable");
    delay(50);
    Firebase.setString(Weather, "/Connection/DEVICE2/UpdateHere", String(WiFi.localIP().toString() + "/update"));
    delay(50);
    TIMEROTA = millis();
  }

  if (StringLineaMeteo == "enable" && DEVICE2OTA != "enable")
  {
    server.begin();
  }
  TIMEZONEINSECONDS = TIMEZONE * 3600;
  timeClient.update();
  timeClient.setTimeOffset(TIMEZONEINSECONDS);
  timeClient.getFullFormattedTime();
  HOURS = timeClient.getHours();
}

void loop()
{
  if (DEVICE2OTA == "enable")
  {
    httpServer.handleClient();
    MDNS.update();
    if (millis() - TIMEROTA > 300000)
    {
      ESP.restart();
    }
  }
  else
  {
    readData();
    writeData();
    gettime();
    Email();
    if (DEVICE2OTA == "enable")
    {
      ESP.restart();
    }
    if (StringLineaMeteo == "enable")
    {
      lineameteo();
    }
  }
}


/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

void update()
{
  String url = "http://otadrive.com/deviceapi/update?";
  url += MakeFirmwareInfo(ProductKey, Version);
  url += "&s=" + String(ESP.getChipId());

  WiFiClient client;
  ESPhttpUpdate.update(client, url, Version);
}
