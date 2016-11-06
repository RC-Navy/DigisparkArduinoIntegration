SBusRx library
==============

**SBusRx** is a library used to receive and extract the 16 proportionnal channels and the 4 flags transported by the SBUS protocol.
SBUS protocol is mainly used with Futaba and FrSky receivers.


Some examples of use cases:
-------------------------
* **Any device connected to the SBUS connector of RC receiver**
* **Custom controller for multi-rotor**
* **Digital data reception over SBUS**

Supported Arduinos:
------------------
All the arduino/micro-controllers equipped with a built-in UART:

* **ATtiny167 (Standalone or Digispark pro)**
* **ATmega368P (UNO)**
* **ATmega32U4 (Arduino Leonardo, Micro and Pro Micro)**
* **ATmega2560 (Arduino MEGA)**

May work with a Software Serial (not tested).

Tip and Tricks:
--------------
Develop your project on an arduino MEGA, Leonardo and then shrink it by loading the sketch in an Micro or Pro Micro or ATtiny167.

API/methods:
-----------
* **SbusRx.serialAttach(Stream *RxStream)**
With:
	* **_RxStream_**: pointer on a serial stream initialized at 100 000 bds, SERIAL_8E2 (eg: &Serial, &Serial1)
 
* **SBusRx.process()**
	* This method shall be called in the main loop to process incoming data


* **uint8_t SBusRx.isSynchro()**:
	* SBUS synchronization indicator: indicates that the SBUS frame has just been received. The channel raw data and/or the channel pulse widths and SBUS flags are available. This is a "clear on read" function (no need to clear explicitely the indicator).

* **uint16_t rawData(uint8_t Ch)**:
	* This method returns the raw data value associated to the requested channel (Ch: 1 to 16). Return value: within [0, 2047].

* **uint16_t width_us(uint8_t Ch)**:
	* This method returns the pulse width (in µs) associated to the requested channel (Ch: 1 to 16). Return value: within [880, 2160].

* **uint8_t  flags(uint8_t FlagId)**
With:
	* **_FlagId_**: the flag identifier among the following choices
	* ** SBUS_RX_CH17**: the status (0/1) of the digital Channel#17
	* ** SBUS_RX_CH18**: the status (0/1) of the digital Channel#18
	* ** SBUS_RX_FRAME_LOST**: the status (0/1) of the frame lost indicator
	* ** SBUS_RX_FAILSAFE**: the status (0/1) of the FailSafe indicator

Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).

