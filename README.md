# LineaMeteoStazione-Personalised-Weather-Station
LineaMeteoStazione, in the improved version, is a complete weather station solar powered which can be interfaced with professional sensors from Sensirion as well as some Davis Instrument component (Rain Gauge, Anemometer)
It also can be interfaced with a less expensive set of anemometer and rain gauge from Misol. It also adds the feature of having a solar radiation measurement and a real UV sensor as well as a very accurate monitoring of the SOC of the battery and email notifications and alert and other services.
The project is aimed as DIY weather station with customized choose of sensors and solar shields and just requiring the assembly part, because the boards will already be given programmed by me as well as the complete PCB. The code will be shared Opensource for the people who wants to try to do it from the beginning or modify it!

Link of the Project
https://www.instructables.com/Personalized-Professional-Weather-Station-LineaMet/


# Features

•	Can be interfaced with Sensirion SHT30/31/35 as a probe or as breakout board with RJ12 cable board adapter included in the kit.

•	Can be interfaced with an extra sensor Sensirion SHT20/21/25 as a probe or as breakout board with RJ12 cable board.

•	Can be interfaced with every rain gauge tipping bucket and it is possible to calibrate and set the resolution of the rain gauge in the settings in the database.

•	Can be interfaced with 2 different type of anemometer, one is an entry level from Misol brand and the other is a Davis Anemometer. The anemometer can be calibrated for accurate Wind direction with the Offset setting.

•	It has an UV REAL index measurement and also a separate Solar Radiation measurement calculated with a very wide Lux range sensor.

•	It has on board an accurate State of Charge of the battery provided by the EZSBC prototype board ESP32, to track in accurate way if the battery need replacement or if it is too low voltage.

•	It has EMAIL ALERTS (Italian or English) as battery alerts and hour or daily report of the weather condition.

•	It has Max and Min Temperature and Humidity alert of the main Temperature and Humidity sensor and alert of UV index, as well as Alert of the second sensor Max and Min Temperature and Humidity and Rain and Rain Intensity Alert and Gust alert.

•	3 seconds time sampling of Wind Speed and Gust and real time Rain intensity and Rain sampling.

•	Sample time or upload time of all the sensors can be modified! Choose between 45 seconds to hours. Usually should not be as frequent as 1 minute to improve battery life and to avoid sampling error of the temperature sensor caused by adjustment of the sensor to the new temperature of the air.

•	Can send data to Wunderground, Weathercloud, Blynk, ThingSpeak and LineaMeteo weather network! The frequency of sending data is also selectable in the menu config for all services in the database.

• Display selectable languages between Italian and English and between metric and imperial unit of measurement of data.


# Testing and Current Consumption

Tested with Arduino IDE 1.8.13, ESP8266 Board Manager version 2.7.4, ESP32 1.0.6 and Attiny 1.0.2

SPECIFICATION DEVICE 1

Attiny85 use 0.5mA in average, when wind activate it can reach 1mA.
Ezsbc Board are great board with current in deep sleep of just 12uA.
DEVICE 1 with all components connected use 0.8mA when sleeping in average and peak of 100mA when waking up.


SPECIFICATION DEVICE 2
Average current of 90mA.

SPECIFICATION DEVICE 3
Deep Sleep of 1mA because of BME680 sensor.
100mA when waking up.
