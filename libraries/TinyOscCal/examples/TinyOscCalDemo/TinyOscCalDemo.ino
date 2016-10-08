/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  |_|  \____/    |_|  \__| |_|  |_|   \__/      |_| 2013

                http://p.loussouarn.free.fr
 English:
 ========
 This sketch demonstrates how to use <TinyOscCal> library  to calibrate the internal oscillator of ATtiny84 and ATtiny85.
 This allows the reliable usage of <SoftSerial> software serial ports on ATtiny84 and ATtiny85.
 Par RC Navy 2012: http://p.loussouarn.free.fr
 
 Francais:
 ========
 Ce sketch montre comment utiliser la librairie <TinyOscCal> pour calibrer l'oscillateur interne des ATtiny84 et ATtiny85.
 Ceci permet l'utilisation fiable des ports series logiciels <SoftSerial> sur les ATtiny84 et ATtiny85.
 Par RC Navy 2012: http://p.loussouarn.free.fr
*/
#include <EEPROM.h>        /* Utilisee par la librairie TinyOscCal */
#include <TinyPinChange.h> /* Utilisee par la librairie SoftSerial */
#include <SoftSerial.h>
#include <TinyOscCal.h>

/*************************************************************************************************************************
                By adjusting the "#define" below, you can play with <TinyOscCal> and <SoftSerial>
*************************************************************************************************************************/

/*
Please note SoftSerial supports to use the same pin for Tx and Rx: very interesting for little chips such as ATtiny85

Hardware Wiring for sharing the same pin for Tx and Rx:
======================================================
                        SERIAL SINGLE I/O
                         DEBUGGING CABLE
            ___________________/\__________________
           /                                       \
                                              ____
 .--------.                                  |    \
 |    GND |--------------------------------+---o5  \
 |        |                           47K  | |   9o |
 |        |                        .--###--' | o4   |
 |  DEBUG |                  4.7K  |         |   8o |
 |  TX_RX |-------------------###--+--|<|------o3   |    ---> To regular RS232 SubD 9 pins Male of PC
 |   PIN  |        ^               | 1N4148  |   7o |         or to true RS232/USB adapter
 |        |        |               '-----------o2   |
 '--------'        |                         |   6o |
  ATtiny85       Single                      | o1  /
 (Digispark)      I/O                        |____/
                                          SubD 9 pins
                                            Female
*/
#define SERIAL_RX_PIN		3 /* Physical Pin 2 for an ATtinyX5 and Physical Pin 10 for an ATtinyX4 */
#define SERIAL_TX_PIN		4 /* Physical Pin 3 for an ATtinyX5 and Physical Pin  9 for an ATtinyX4 */

#define SERIAL_BAUD_RATE	19200 /* Adjust here the desired Serial Baud Rate */

#define FORCE_CAL		0    /* Change this to 1 to force a re-calibration (this will clear the EEPROM area used by <TinyOscCal>) */

#define EEPROM_OSCCAL_LOCATION  0    /* OSCCAL offset value is stored at index 0 in EEPROM */

#define DIRECT_CNX_TO_RS232	true /* Change this to false if you are using a MAX232 line driver or a TTL to USB converter */

SoftSerial MySerial(SERIAL_RX_PIN, SERIAL_TX_PIN, DIRECT_CNX_TO_RS232);

void setup()
{
  MySerial.begin(SERIAL_BAUD_RATE); /* IMPORTANT: init the serial BEFORE calling TinyOscCal.init() */
  TinyOscCal.init(&MySerial, EEPROM_OSCCAL_LOCATION, FORCE_CAL); /* Calibrate internal oscillator if necessary */
#if (SERIAL_RX_PIN == SERIAL_TX_PIN)
  MySerial.txMode(); /* Mandatory if SERIAL_RX_PIN and SERIAL_RX_PIN share the same pin */
#endif
  MySerial.print(F("Byte used in EEPROM for <TinyOscCal> library: "));MySerial.println((int)TinyOscCal.getEepromStorageSize());
  MySerial.print(F("The OSCCAL offset stored in EEPROM is: "));MySerial.println((int8_t)EEPROM.read(EEPROM_OSCCAL_LOCATION));
}

void loop()
{
static uint16_t Sec = 0;
static uint32_t StartMs = millis();

  if((millis() - StartMs) >= 1000UL)
  {
    StartMs = millis();
    Sec++;
    MySerial.print(F("Uptime: "));MySerial.print(Sec);MySerial.println(F(" sec"));
  }
}
