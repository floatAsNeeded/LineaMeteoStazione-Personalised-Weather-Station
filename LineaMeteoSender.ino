///////////////////////////////LIBRARY DECLARATION////////////////////////////////////////
#include <Arduino.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <FirebaseESP32.h>       //https://github.com/mobizt/Firebase-ESP32
#include "ClosedCube_SHT31D.h"   //https://github.com/closedcube/ClosedCube_SHT31D_Arduino
#include "Adafruit_VEML6075.h"   //https://github.com/adafruit/Adafruit_VEML6075
#include "DFRobot_SHT20.h"       //https://github.com/DFRobot/DFRobot_SHT20
#include "LC709203F.h"           //https://github.com/EzSBC/ESP32_Bat_Pro/blob/main/LC709203F.zip
#include "Max44009.h"            //https://github.com/RobTillaart/Max44009
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <Wire.h>
WiFiManager wifiManager;
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPUpdateServer.h>
#include <WiFiClient.h>

//OTA REMOTE//
// To inject firmware info into binary file, You have to use following macro according to let
// OTAdrive to detect binary info automatically
#define ProductKey "" // Replace with your own APIkey
#define Version "1.0.1.2"
#define MakeFirmwareInfo(k, v) "&_FirmwareInfo&k=" k "&v=" v "&FirmwareInfo_&"
void update();

//OTA LOCAL//
WebServer httpServer(80);
HTTPUpdateServer httpUpdater;
const char* host = "esp32-webupdate";
String DEVICE1OTA;
unsigned long TIMEROTA;
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
#define FIREBASE_HOST ""                 // the project name address from firebase id
#define FIREBASE_AUTH ""            // the secret key generated from firebase
FirebaseData Weather;

//WIND DIRECTION//
#define WindDirectionDavis 32 // anemometer
#define WindDirectionMisol 33 // anemometer

////////////RAIN////////////////
byte PluvioFlag = 0; // detect interrupt of rain
byte rainrateI2C = 0;
byte rainrateMaxI2C = 0;
byte rainrateMaxI2CIntensity = 0;

////////////WIND/////////////////
int WindDirectionDavisValue;
int WindDirectionMisolValue;
byte Rotations;
byte GustRotations;

/////TIMING VARIALABLES//////
unsigned long timeout; // waiting for WiFi connection
bool res; // WiFi Status
unsigned int sampletime;
byte ReadByte = 0;
byte WiFiReset = 0;

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
byte UVDETECT = 0;

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
  if (UVDETECT == 0)
  {
    if (! uv.begin()) {
      UVDETECT = 1;
    }
  }
  if (UVDETECT == 0)
  {
    uv.setIntegrationTime(VEML6075_100MS);
    uv.setHighDynamic(false);
    uv.setForcedMode(false);
    uv.setCoefficients(2.22, 1.17,  // UVA_A and UVA_B coefficients
                       2.95, 1.58,  // UVB_C and UVB_D coefficients
                       0.004770, 0.006135); // UVA and UVB responses
  }
  myLux.setAutomaticMode();
}

void initData()
{
  if (Firebase.getInt(Weather, "/Connection/DEVICE1/ResetWiFi"))
  {
    WiFiReset = Weather.to<int>();
  }
  if (WiFiReset == 1)
  {
    WiFiReset = 0;
    Firebase.setInt(Weather, "/Connection/DEVICE1/ResetWiFi", 0);
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
        res = wifiManager.autoConnect("LineaMeteoStazioneS", "LaMeteo2005");

        if (!res) {
          esp_sleep_enable_timer_wakeup(300 * 1000000LL);
          esp_deep_sleep_start();
        }
      }
    }
  }
  Firebase.deleteNode(Weather, "/Connection/DEVICE1/UpdateHere");
  Firebase.setString(Weather, "/Connection/DEVICE1/Version", Version);

  if (Firebase.getInt(Weather, "/Time/SampleTime"))
  {
    sampletime = Weather.to<int>();
  }
  if (sampletime == 0)
  {
    sampletime = 90;
  }
  PluvioFlag = readTiny(I2CSlaveAddress);
  Rotations = readTiny(I2CSlaveAddress);
  GustRotations = readTiny(I2CSlaveAddress);
  rainrateI2C = readTiny(I2CSlaveAddress);
  rainrateMaxI2C = readTiny(I2CSlaveAddress);
  rainrateMaxI2CIntensity = readTiny(I2CSlaveAddress);
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
        esp_sleep_enable_timer_wakeup(300 * 1000000LL);
        esp_deep_sleep_start();
      }
    }
  }
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   // connect to firebase
  //Firebase.reconnectWiFi(true);
  Firebase.setMaxRetry(Weather, 2);

  if (Firebase.getString(Weather, "/Connection/DEVICE1/Update"))
  {
    DEVICE1OTA = Weather.to<const char *>();
  }
  if (DEVICE1OTA == "enable")
  {
    Firebase.setString(Weather, "/Connection/DEVICE1/Update", "disable");
    delay(50);
    update();
  }

  if (DEVICE1OTA == "enable")
  {
    MDNS.begin(host);
    if (MDNS.begin("esp32")) {
      Serial.println("mDNS responder started");
    }
    httpUpdater.setup(&httpServer);
    httpServer.begin();

    MDNS.addService("http", "tcp", 80);
    Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
    Firebase.setString(Weather, "/Connection/DEVICE1/Update", "disable");
    delay(100);
    Firebase.setString(Weather, "/Connection/DEVICE1/UpdateHere", String(WiFi.localIP().toString() + "/update"));
    delay(100);
    TIMEROTA = millis();
  }
  else
  {
    initData();
  }
}


void loop()
{
  if (DEVICE1OTA == "enable")
  {
    httpServer.handleClient();
    if (millis() - TIMEROTA > 300000)
    {
      ESP.restart();
    }
  }
  else
  {
    ReadSensors();
    WriteData();
    if (UVDETECT == 0)
    {
      uv.setForcedMode(true);
    }
    Firebase.setInt(Weather, "Time/Communication/DataSent", 1);
    esp_sleep_enable_timer_wakeup(sampletime * 1000000LL);
    esp_deep_sleep_start();
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
  WindDirectionMisolValue = analogRead(WindDirectionMisol);
  WindDirectionDavisValue = analogRead(WindDirectionDavis);
  Firebase.setInt(Weather, "Time/Communication/WindDirectionDavis", WindDirectionDavisValue);
  Firebase.setInt(Weather, "Time/Communication/WindDirectionMisol", WindDirectionMisolValue);
  Firebase.setInt(Weather, "/Connection/DEVICE1/WiFiSignaldb",   WiFi.RSSI());
  Firebase.setFloat(Weather, "/Battery/CellVoltage", gg.cellVoltage_mV() / 1000.0);
  Firebase.setInt(Weather, "/Battery/CellPercentage", gg.cellRemainingPercent10() / 10);
  if (humiditySHT2x != 998)
  {
    Firebase.setFloat(Weather, "/SHT2x/Temperature/Temperature", tempSHT2x);
    Firebase.setInt(Weather, "/SHT2x/Humidity/Humidity", humiditySHT2x);
  }
  Firebase.setFloat(Weather, "/SHT3x/Temperature/Temperature", temperatureSHT3x);
  Firebase.setInt(Weather, "/SHT3x/Humidity/Humidity", humiditySHT3x);

  if (UVDETECT == 0)
  {
    UVindex = uv.readUVI();
    if (UVindex < 0)
    {
      Firebase.setFloat(Weather, "/Light/UVindex", 0);
    }
    else
    {
      Firebase.setFloat(Weather, "/Light/UVindex", UVindex);
    }
  }
  Firebase.setFloat(Weather, "/Light/Lux", myLux.getLux());
  Firebase.setInt(Weather, "/Time/Communication/PluvioFlag", PluvioFlag);
  Firebase.setInt(Weather, "/Time/Communication/rainrateI2C", rainrateI2C);
  Firebase.setInt(Weather, "/Time/Communication/rainrateMaxI2C", rainrateMaxI2C);
  Firebase.setInt(Weather, "/Time/Communication/rainrateMaxI2CIntensity", rainrateMaxI2CIntensity);
  Firebase.setInt(Weather, "/Time/Communication/Rotations", Rotations);
  Firebase.setInt(Weather, "/Time/Communication/GustRotations", GustRotations);
}

String getChipId()
{
  String ChipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
  ChipIdHex += String((uint32_t)ESP.getEfuseMac(), HEX);
  return ChipIdHex;
}

void update()
{
  String url = "http://otadrive.com/deviceapi/update?";
  url += MakeFirmwareInfo(ProductKey, Version);
  url += "&s=" + getChipId();

  WiFiClient client;
  httpUpdate.update(client, url, Version);
}
