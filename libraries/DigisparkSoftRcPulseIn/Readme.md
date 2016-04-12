SoftRcPulseIn library
======================

**SoftRcPulseIn** is an asynchronous library designed to read RC pulse signals. It is a non-blocking version of arduino **pulseIn()** function.

Some examples of use cases:
-------------------------
* **RC Servo/ESC/Brushless Controller**
* **Multi-switch (RC Channel to digital outputs converter)** (look at **RcSeq** library)
* **Servo sequencer** (look at **RcSeq** library which uses **SoftRcPulseOut** library)
* **RC Robot using wheels with modified Servo to support 360° rotation**
* **RC pulse stretcher** (in conjunction with **SoftRcPulseOut** library)

Supported Arduinos:
------------------
* **ATmega328 (UNO)**
* **ATmega2560 (MEGA)**
* **ATtiny84 (Standalone)**
* **ATtiny85 (Standalone or Digispark)**
* **ATtiny167 (Digispark pro)**
* **ATmega32U4 (Leonardo, Micro, Pro Micro)**

Tip and Tricks:
--------------
Develop your project on an arduino UNO or MEGA, and then shrink it by loading the sketch in an ATtiny or Digispark (pro).

API/methods:
-----------
* attach()
* available()
* width_us()
* timeout()

* Constants for version management:
	* **SOFT_RC_PULSE_IN_VERSION**: returns the library version
	* **SOFT_RC_PULSE_IN_REVISION**: returns the library revision

Design considerations:
---------------------
The **SoftRcPulseIn** library relies the **TinyPinChange** library. This one shall be included in the sketch as well.

On the arduino MEGA (ATmega2560), as all the pins do not support "pin change interrupt", only the following pins are supported:

* 10 -> 15
* 50 -> 53
* A8 -> A15

On the arduino Lenardo, Micro and Pro Micro (ATmega32U4), as all the pins do not support "pin change interrupt", only the following pins are supported:

* 0  -> 3  (external INT0, INT1, INT2 and INT3 are used as emulated Pin Change Interrupt)
* 8  -> 11 (pin 11 is not available on the headers/connectors)
* 14 -> 17 (pin 17 is not available on the headers/connectors)

On other devices (ATmega328, ATtiny84, ATtiny85 and ATtiny167), all the pins are usable.

Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).

