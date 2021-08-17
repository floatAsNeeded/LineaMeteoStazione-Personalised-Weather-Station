///////////////////////////////LIBRARY DECLARATION////////////////////////////////////////
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "ClosedCube_SHT31D.h"
#include "Adafruit_VEML6075.h"
#include "DFRobot_SHT20.h"
#include "LC709203F.h"
#include "Max44009.h"
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <Wire.h>
WiFiManager wifiManager;

//--------------------------------------------------------------------------------------//

///BATTERY OBJECT//
LC709203F gg;  // create a gas gauge object.

//DECLARATION SENSORS//
ClosedCube_SHT31D sht3xd;
DFRobot_SHT20    sht20;

//LIGHT SENSOR DEFINE//
Adafruit_VEML6075 uv = Adafruit_VEML6075();
Max44009 myLux(0x4A);

//I2C COMUNICATION ADDRESS//
const int I2CSlaveAddress = 8;      // I2C Address.

////////////////////*********FIREBASE DETAILS************///////////////////////////////////
#define FIREBASE_HOST "weather-station-95094.firebaseio.com"                 // the project name address from firebase id
#define FIREBASE_AUTH "PihHbkRGMXvTPJ7vmJdQmR3wfSvIZPZhJpL86tU0"            // the secret key generated from firebase
//#define FIREBASE_HOST "trweather-station-default-rtdb.firebaseio.com"                 // the project name address from firebase id
//#define FIREBASE_AUTH "C0tnTKAzihx9uz7sZ5BiALs8tVeTJBMN9TGK91cR"            // the secret key generated from firebase
FirebaseData Weather;

//WIND DIRECTION//
#define WindDirectionDavis 32 // anemometer
#define WindDirectionAnemometer 33 // anemometer

////////////RAIN////////////////
float mmGoccia; // tipping bucket count in mm
float mmPioggia = 0.0; // daily rain
RTC_DATA_ATTR float mmPioggia24H = 0;
RTC_DATA_ATTR float rainrate = 0; // real-time rainrate
RTC_DATA_ATTR float rainrateMax = 0; // daily rainrateMax
byte rainratedetect = 0;
int rainrateMaxIntensity = 0;
byte PluvioFlag = 0; // detect interrupt of rain
byte rainrateI2C = 0;
byte rainrateMaxI2C = 0;
byte rainrateMaxI2CIntensity = 0;

////////////WIND/////////////////
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

/////TIMING VARIALABLES//////
unsigned long timeout; // waiting for WiFi connection
byte CurrentDay; // current day to reset max and min values
RTC_DATA_ATTR byte PreviousDay; // previous day to reset max and min values
bool res; // WiFi Status
unsigned int sampletime;
RTC_DATA_ATTR byte INITDATA = 0;
byte ReadByte = 0;
RTC_DATA_ATTR byte Alternate = 0;
RTC_DATA_ATTR boolean justrestart = true;

//////////TEMPERATURE///////////
//SHT2X//
float tempSHT2x; // temperature in centigrade
float temp_f; // temperature in fahrenheit
int humiditySHT2x; // humidity variable

//SHT3X//
float temperatureSHT3x; // temperature in centigrade
int humiditySHT3x; // humidity variable

//////////////UV/////////////////
float UVindex; // variable UVindex

void InitSensors()
{
  sht20.initSHT20();                                  // Init SHT20 Sensor
  delay(100);
  sht20.checkSHT20();
  sht3xd.begin(0x44);
  sht3xd.readSerialNumber();
  if (!gg.begin()) {
    while (1) delay(10);
  }
  gg.setCellCapacity(LC709203F_APA_3000MAH);
  //gg.setAlarmVoltage(3.4);
  gg.setCellProfile( LC709203_NOM3p7_Charge4p2 ) ;
  uv.begin();
  uv.setIntegrationTime(VEML6075_100MS);
  uv.setHighDynamic(false);
  uv.setForcedMode(false);
  uv.setCoefficients(2.22, 1.17,  // UVA_A and UVA_B coefficients
                     2.95, 1.58,  // UVB_C and UVB_D coefficients
                     0.004770, 0.006135); // UVA and UVB responses

  myLux.setAutomaticMode();
}

void initData()
{
  if (INITDATA == 0)
  {
    if (Firebase.getFloat(Weather, "/Rain/Rain24H"))
    {
      mmPioggia24H = Weather.floatData();
    }
    else
    {
      //Serial.println(Weather.errorReason());
    }
    if (Firebase.getFloat(Weather, "/Rain/RainRateMax"))
    {
      rainrateMax = Weather.floatData();
    }
    if (Firebase.getInt(Weather, "Time/PreviousDay"))
    {
      PreviousDay = Weather.intData();
    }
    INITDATA = 1;
  }
  if (Firebase.getInt(Weather, "Time/SampleTime"))
  {
    sampletime = Weather.intData();
  }
  if (sampletime == 0)
  {
    sampletime = 90;
  }
  if (Firebase.getInt(Weather, "Wind/Anemometer"))
  {
    AnemometerType = Weather.intData();
  }
  if (Firebase.getFloat(Weather, "/Rain/mmGoccia"))
  {
    mmGoccia = Weather.floatData();
  }
  PluvioFlag = readTiny(I2CSlaveAddress);
  Rotations = readTiny(I2CSlaveAddress);
  GustRotations = readTiny(I2CSlaveAddress);
  rainrateI2C = readTiny(I2CSlaveAddress);
  rainrateMaxI2C = readTiny(I2CSlaveAddress);
  rainrateMaxI2CIntensity = readTiny(I2CSlaveAddress);
}

void setup()
{
  Wire.begin(); //SDA , SCL
  //Serial.begin(115200);
  InitSensors();
  WiFi.mode(WIFI_STA);
  wifiManager.setConfigPortalTimeout(300);
  WiFi.begin();
  timeout = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    //Serial.print(".");
    if (millis() - timeout > 25000)
    {
      res = wifiManager.autoConnect("LineaMeteoStazioneS", "LaMeteo2005");

      if (!res) {
        esp_sleep_enable_timer_wakeup(60 * 1000000LL);
        esp_deep_sleep_start();
      }
    }
  }
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   // connect to firebase
  Firebase.setMaxRetry(Weather, 2);
  initData();
}


void loop()
{
  readRain ();
  if (AnemometerType == 1)
  {
    readDavisAnemometer ();
  }
  else if (AnemometerType == 2)
  {
    readAnemometer ();
  }
  ReadSensors();
  WriteData();
  CheckDay();
  uv.setForcedMode(true);
  esp_sleep_enable_timer_wakeup(sampletime * 1000000LL);
  esp_deep_sleep_start();
}

byte readTiny(int address) {
  byte rawRead;
  switch (ReadByte) {
    case 0:
      Wire.requestFrom(address, 1);                  // The TinyWire library only allows for one byte to be requested at a time
      while (Wire.available() == 0)  Serial.print("W");
      rawRead = Wire.read();
      return rawRead;
      ReadByte = 1;
      break;
    case 1:
      Wire.requestFrom(address, 1);                  // The TinyWire library only allows for one byte to be requested at a time
      while (Wire.available() == 0)  Serial.print("W");
      rawRead = Wire.read();
      return rawRead;
      ReadByte = 2;
      break;
    case 2:
      Wire.requestFrom(address, 1);                  // The TinyWire library only allows for one byte to be requested at a time
      while (Wire.available() == 0)  Serial.print("W");
      rawRead = Wire.read();
      return rawRead;
      ReadByte = 3;
      break;
    case 3:
      Wire.requestFrom(address, 1);                  // The TinyWire library only allows for one byte to be requested at a time
      while (Wire.available() == 0)  Serial.print("W");
      rawRead = Wire.read();
      return rawRead;
      ReadByte = 4;
      break;
    case 4:
      Wire.requestFrom(address, 1);                  // The TinyWire library only allows for one byte to be requested at a time
      while (Wire.available() == 0)  Serial.print("W");
      rawRead = Wire.read();
      return rawRead;
      ReadByte = 5;
      break;
    case 5:
      Wire.requestFrom(address, 1);                  // The TinyWire library only allows for one byte to be requested at a time
      while (Wire.available() == 0)  Serial.print("W");
      rawRead = Wire.read();
      return rawRead;
      ReadByte = 0;
      break;
  }
}

//RAIN//
void readRain () {
  mmPioggia = (mmPioggia + mmGoccia) * PluvioFlag;
  mmPioggia24H = mmPioggia24H + mmPioggia;
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
}

//WIND//
void readDavisAnemometer () {
  //FORMULA V=P(Rotations)(2.25/T)--->constant
  constant = 2.25 / (average / 1000);
  WindSpeed = (Rotations * constant) * 1.60934;
  GustNow = (GustRotations * constant) * 1.60934;

  VaneValue = analogRead(WindDirectionDavis);
  Direction = map(VaneValue, 0, 4095, 0, 360);
  CalDirection = Direction;
}

//WIND//
void readAnemometer () {
  WindSpeed = (Rotations * 2448.0) / average;
  GustNow = (GustRotations * 2448.0) / average;


  VaneValue = analogRead(WindDirectionAnemometer);

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

//READ SHT3X//
void printResult(String text, SHT31D result) {
  if (result.error == SHT3XD_NO_ERROR) {
    temperatureSHT3x = result.t;
    humiditySHT3x = result.rh;
  } else {
    //Serial.print(text);
    //Serial.print(": [ERROR] Code #");
    //Serial.println(result.error);
  }
}

void ReadSensors()
{
  humiditySHT2x = sht20.readHumidity();              // Read Humidity
  tempSHT2x = sht20.readTemperature();               // Read Temperature
  printResult("Pooling Mode", sht3xd.readTempAndHumidity(SHT3XD_REPEATABILITY_HIGH, SHT3XD_MODE_POLLING, 50));
}

void WriteData()
{
  Alternate++;
  if (justrestart == true)
  {
    justrestart = false;
    Firebase.setInt(Weather, "Connection/WiFiSignaldb",   WiFi.RSSI());
    Firebase.setFloat(Weather, "Battery/CellVoltage", gg.cellVoltage_mV() / 1000.0);
    Firebase.setInt(Weather, "Battery/CellPercentage", gg.cellRemainingPercent10() / 10);
  }
  if (Alternate == 5)
  {
    justrestart = true;
    Alternate = 0;
  }
  Firebase.setFloat(Weather, "SHT2x/Temperature/Temperature", tempSHT2x);
  Firebase.setInt(Weather, "SHT2x/Humidity/Humidity", humiditySHT2x);
  Firebase.setFloat(Weather, "SHT3x/Temperature/Temperature", temperatureSHT3x);
  Firebase.setInt(Weather, "SHT3x/Humidity/Humidity", humiditySHT3x);
  Firebase.setFloat(Weather, "Rain/RainRate", rainrate);
  Firebase.setFloat(Weather, "Wind/WindSpeed", WindSpeed);
  Firebase.setInt(Weather, "Wind/WindDirection", CalDirection);
  Firebase.setFloat(Weather, "Wind/Gust", GustNow);
  UVindex = uv.readUVI();
  if (UVindex < 0)
  {
    Firebase.setFloat(Weather, "Light/UVindex", 0);
  }
  else
  {
    Firebase.setFloat(Weather, "Light/UVindex", UVindex);
  }
  Firebase.setFloat(Weather, "Light/Lux", myLux.getLux());

  if (PluvioFlag > 0)
  {
    Firebase.setFloat(Weather, "Rain/Rain24H", mmPioggia24H);
  }
  if (rainratedetect == 1)
  {
    Firebase.setFloat(Weather, "Rain/RainRateMax", rainrateMax);
    rainratedetect = 0;
  }
}

void CheckDay()
{
  if (Firebase.getInt(Weather, "Time/CurrentDay"))
  {
    CurrentDay = Weather.intData();
  }

  if (CurrentDay != PreviousDay)
  {
    PreviousDay = CurrentDay;
    mmPioggia24H = 0;
    rainrateMax = 0;
    Firebase.setInt(Weather, "Time/PreviousDay",  PreviousDay);
    Firebase.setFloat(Weather, "Rain/Rain24H", mmPioggia24H);
    Firebase.setFloat(Weather, "Rain/RainRateMax", rainrateMax);
  }
}
