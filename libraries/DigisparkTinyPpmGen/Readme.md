TinyPpmGen library
==================

**TinyPpmGen** is an interrupt-driven RC PPM generator library using a 8 bit Timer Output Compare Interrupt. As this library uses hardware resources, the timing is very accurate but the PPM output pin is imposed (cf. _Design Considerations_ below).

This PPM generator can transport up to 12 RC channels and supports positive and negative PPM modulation. The PPM frame period is constant (configurable from 10 to 40ms, default=20 ms) regardless of the channel pulse widths.

Some examples of use cases:
-------------------------
* **Standalone RC PPM generator**
* **Channel substitution and/or addition in an existing PPM frame**
* **Digital data transmission over PPM**

Supported Arduinos:
------------------
* **ATtiny167 (Standalone or Digispark pro)**
* **ATtiny85 (Standalone or Digispark)**
* **ATtiny84 (Standalone)**
* **ATmega368P (UNO)**
* **ATmega32U4 (Arduino Leonardo, Micro and Pro Micro)**

Tip and Tricks:
--------------
Develop your project on an arduino UNO, Leonardo, Micro or Pro Micro and then shrink it by loading the sketch in an ATtiny or Digispark (pro).

API/methods:
-----------
* **TinyPpmGen.begin(uint8_t _PpmModu_, uint8_t _ChNb_, uint16_t _PpmPeriod_us=20000_)**
With:
	* **_PpmModu_**: **TINY_PPM_GEN_POS_MOD** or **TINY_PPM_GEN_NEG_MOD** for respectiveley positive and negative PPM modulation
	* **_ChNb_**: The number of RC channel to transport in the PPM frame (1 to 12)
	* **PpmPeriod_us**: PPM period in µs (from 10000 to 40000 µs, 20000 µs if 3rd argument absent)
 
* **TinyPpmGen.setChWidth_us(uint8_t _Ch_, uint16_t _Width_us_)**
With:
	* **_Ch_**: the RC channel (1 to _ChNb_)
	* **_Width_us_**: the pulse width in µs

* **uint8_t TinyPpmGen.isSynchro()**:
	* PPM Synchronization indicator: indicates that the pulse values have just been recorded for the current PPM frame generation and gives 20 ms for preparing next pulse widths. This allows to pass digital information over PPM (one different pulse width per PPM frame). This is a "clear on read" function (no need to clear explicitely the indicator).

* **void TinyPpmGen.suspend()**:
	* Suspends the PPM frame generation.

* **void TinyPpmGen.resume()**:
	* Resumes the PPM frame generation.

Design considerations:
---------------------
As this library relies on Timer Output Compare capabilities, the PPM output pin is imposed by the hardware and is target dependent.

However, there is some flexibility as the timer and the channel can be chosen by the user (in TinyPpmGen.h):

* **ATtiny167** (Digispark pro):
	* TIMER(0), CHANNEL(A) -> OC0A -> PB0 -> Pin#0

* **ATtiny85**   (Digispark):
	* TIMER(0), CHANNEL(A) -> OC0A -> PB0 -> Pin#0
	* TIMER(0), CHANNEL(B) -> OC0B -> PB1 -> Pin#1
	* TIMER(1), CHANNEL(A) -> OC1A -> PB1 -> Pin#1

* **ATtiny84**   (Standalone):
	* TIMER(0), CHANNEL(A) -> OC0A -> PB2 -> Pin#5 | Digital 8 : D8
	* TIMER(0), CHANNEL(B) -> OC0B -> PA7 -> Pin#6 | Digital 7 : D7

* **ATmega328P** (Arduino UNO):
	* TIMER(0), CHANNEL(A) -> OC0A -> PD6 -> Pin#6
	* TIMER(0), CHANNEL(B) -> OC0B -> PD5 -> Pin#5
	* TIMER(2), CHANNEL(A) -> OC2A -> PB3 -> Pin#11
	* TIMER(2), CHANNEL(B) -> OC2B -> PD3 -> Pin#3

* **ATmega32U4** (Arduino Leonardo, Micro and Pro Micro):
	* TIMER(0), CHANNEL(A) -> OC0A -> PB7 -> Pin#11 (pin not available on connector of Pro Micro)
	* TIMER(0), CHANNEL(B) -> OC0B -> PD0 -> Pin#3

Contact
-------

If you have some ideas of enhancement, please contact me by clicking on: [RC Navy](http://p.loussouarn.free.fr/contact.html).

