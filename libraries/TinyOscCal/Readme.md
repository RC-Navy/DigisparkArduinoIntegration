TinyOscCal library
==================

**TinyOscCal** allows to easily calibrate the internal RC oscillator of ATtint85 and ATtiny84 by using a **SoftSerial** serial port connected to a PC.

How does it work?
----------------
**TinyOscCal** sends messages on a **SoftSerial** serial port by trying incremental values of **OSCCAL** register. The messages will be correct in the Serial Terminal on the PC for contiguous **OSCCAL** values and then will be incorrect for the last values.
Then, restart the ATtiny, and hit "Enter" in the PC when you reached the middle of the correct **OSCCAL** values area: the current **OSCCAL** value will be automatically memorized in the EEPROM at the location defined in the init() method.

For additional information, please, look at the example folder provided with the **TinyOscCal** library.

Supported Arduinos:
------------------
* **ATtiny85 (Standalone or Digispark)**
* **ATtiny84 (Standalone)**


API/methods:
-----------
* init()
* getEepromStorageSize()


Design considerations:
---------------------
The **TinyOscCal** library only works with the **SoftSerial** library. This one shall be included in the sketch as well.


Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).

