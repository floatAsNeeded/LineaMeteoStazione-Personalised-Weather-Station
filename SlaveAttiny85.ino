#include <TinyWireS.h>       // Requires fork by Rambo with onRequest support
//#include <avr/wdt.h>         // watchdog
#include <avr/sleep.h>
#include "PinChangeInterrupt.h"
// Requires headers for AVR defines and ISR function
//#include <avr/io.h>
//#include <avr/interrupt.h>

// Choose a valid PinChangeInterrupt pin of your Arduino board
#define INT_PINRAIN PB4
#define INT_PINANEMOMETER PB1
const int I2CSlaveAddress = 8;      // I2C Address.
byte ReadByte = 0;


////////////RAIN////////////////
volatile byte PluvioFlag = 0; // detect interrupt of rain
volatile byte RainFlag = 0; // detect interrupt of rain
volatile unsigned long ContactBounceTimeRain = 0; // Timer to avoid double counting in interrupt
float mmGoccia = 0.3; // tipping bucket count in mm
unsigned int gocce = 0; // tipping bucket movements
volatile unsigned long time1; // rain rate timing calculation
unsigned long PluvioStep = 0; // rain rate timing calculation
unsigned long PluvioOldStep = 0; // rain rate timing calculation
float rainrate = 0; // real-time rainrate
float rainrateMax = 0;
byte rainrateI2C = 0;
byte rainrateMaxI2C = 0;
byte rainrateMaxI2CIntensity = 0;

////////////WIND/////////////////
volatile byte Rotations; // cup rotation counter used in interrupt routine
volatile unsigned long ContactBounceTime = 0;
unsigned long calculation = 0; //millis() for sample time
unsigned int average = 3000; // sample time for wind speed
byte Gust; // gust variable


void setup() {
  sleepNow();
  //wdt_enable(WDTO_8S);               // Watchdog
  //pinMode(LED_PIN, OUTPUT);
  //cli();                            // Disable interrupts during setup
  //PCMSK |= (1 << INTERRUPT_PINRAIN);    // Enable interrupt handler (ISR) for our chosen interrupt pin (PCINT1/PB1/pin 6)
  //GIMSK |= (1 << PCIE);             // Enable PCINT interrupt in the general interrupt mask
  pinMode(INT_PINRAIN, INPUT);   // Set our interrupt pin as input with a pullup to keep it stable
  pinMode(INT_PINANEMOMETER, INPUT);   // Set our interrupt pin as input with a pullup to keep it stable
  //sei();                            //last line of setup - enable interrupts after setup
  attachPCINT(digitalPinToPCINT(INT_PINRAIN), RAIN, FALLING);
  attachPCINT(digitalPinToPCINT(INT_PINANEMOMETER), WIND, FALLING);
  TinyWireS.begin(I2CSlaveAddress);      // Begin I2C Communication
  TinyWireS.onRequest(transmit);         // When requested, call function transmit()
}


void loop() {
  sleepNow();
  if (millis() - calculation >= average)
  {
    calculation = millis();
    Rotations = 0;
  }
  if (Rotations > Gust)
  {
    Gust = Rotations;
  }
  if (RainFlag == 1) {
    RainFlag = 0;
    PluvioOldStep = PluvioStep;
    PluvioStep = time1;
    gocce++; // incrementa numero basculate
  }
  PluvioDataEngine();
  //wdt_reset();                          // feed the watchdog
  //delay(10);
}


//-------------------------------------------------------------------

void transmit() {
  switch (ReadByte) {
    case 0:
      TinyWireS.send(PluvioFlag);
      PluvioFlag = 0;
      ReadByte = 1;
      break;
    case 1:
      TinyWireS.send(Rotations);
      ReadByte = 2;
      break;
    case 2:
      TinyWireS.send(Gust);
      Gust = 0;
      ReadByte = 3;
      break;
    case 3:
      TinyWireS.send(rainrateI2C);
      ReadByte = 4;
      break;
    case 4:
      TinyWireS.send(rainrateMaxI2C);
      rainrateMaxI2C = 0;
      ReadByte = 5;
      break;
    case 5:
      TinyWireS.send(rainrateMaxI2CIntensity);
      rainrateMaxI2CIntensity = 0;
      ReadByte = 0;
      break;
  }
}

void RAIN()
{
  if ((millis() - ContactBounceTimeRain) > 250 ) { // debounce the switch contact.
    PluvioFlag++;
    RainFlag = 1;
    ContactBounceTimeRain = millis();
    time1 = millis();
  }
}

void WIND()
{
  if ((millis() - ContactBounceTime) > 15 ) { // debounce the switch contact.
    Rotations++;
    ContactBounceTime = millis();
  }
}

void PluvioDataEngine() {
  if (((PluvioStep - PluvioOldStep) != 0) && (gocce >= 2)) {
    if ((millis() - PluvioStep) > (PluvioStep - PluvioOldStep)) {
      rainrate = 3600 / (((millis() - PluvioStep) / 1000)) * mmGoccia;
      rainrateMax = rainrate / 10;
      if (rainrate < 1) {
        gocce = 0;
        rainrate = 0;
      }
    } else {
      rainrate = 3600 / (((PluvioStep - PluvioOldStep) / 1000)) * mmGoccia;
      rainrateMax = rainrate / 10;
    }
  } else {
    rainrate = 0.0;
  }
  if (rainrate > 255)
  {
    rainrateI2C = 255;
  }
  else
  {
    rainrateI2C = (byte)rainrate;
  }
  if (rainrateI2C > rainrateMaxI2C)
  {
    rainrateMaxI2C = rainrateI2C;
  }
  if (rainrateMax > rainrateMaxI2CIntensity)
  {
    rainrateMaxI2CIntensity = (byte)rainrateMax;
  }
}

void sleepNow()         // here we put the arduino to sleep
{
  /* Now is the time to set the sleep mode. In the Atmega8 datasheet
     http://www.atmel.com/dyn/resources/prod_documents/doc2486.pdf on page 35
     there is a list of sleep modes which explains which clocks and
     wake up sources are available in which sleep modus.

     In the avr/sleep.h file, the call names of these sleep modus are to be found:

     The 5 different modes are:
         SLEEP_MODE_IDLE         -the least power savings
         SLEEP_MODE_ADC
         SLEEP_MODE_PWR_SAVE
         SLEEP_MODE_STANDBY
         SLEEP_MODE_PWR_DOWN     -the most power savings

     For now, we want as much power savings as possible, so we
     choose the according
     sleep modus: SLEEP_MODE_PWR_DOWN

     Timer 2 overflow interrupt is only able to wake up the ATmega in PWR_SAVE

  */
  // disable ADC
  ADCSRA = 0;
  set_sleep_mode(SLEEP_MODE_IDLE);   // sleep mode is set here

  sleep_enable();          // enables the sleep bit in the mcucr register
  // so sleep is possible. just a safety pin

  sleep_mode();            // here the device is actually put to sleep!!
  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
  sleep_disable();         // first thing after waking from sleep:
  // disable sleep...
}
