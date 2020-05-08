TinyCppmGen library
==================

**TinyCppmGen** is an interrupt-driven RC CPPM generator library using a 8 bit Timer Output Compare Interrupt. As this library uses hardware resources, the timing is very accurate but the CPPM output pin is imposed (cf. _Design Considerations_ below).

This CPPM generator can transport up to 12 RC channels and supports positive and negative CPPM modulation. The CPPM frame period is constant (configurable from 10 to 40ms, default=20 ms) regardless of the channel pulse widths.

Some examples of use cases:
-------------------------
* **Standalone RC CPPM generator**
* **Channel substitution and/or addition in an existing CPPM frame**
* **Digital data transmission over CPPM**

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
* **TinyCppmGen.begin(uint8_t _CppmModu_, uint8_t _ChNb_, uint16_t _CppmPeriod_us=20000_)**
With:
	* **_CppmModu_**: **TINY_CPPM_GEN_POS_MOD** or **TINY_CPPM_GEN_NEG_MOD** for respectiveley positive and negative CPPM modulation
	* **_ChNb_**: The number of RC channel to transport in the CPPM frame (1 to 12)
	* **CppmPeriod_us**: CPPM period in µs (from 10000 to 40000 µs, 20000 µs if 3rd argument absent)
 
* **TinyCppmGen.setChWidth_us(uint8_t _Ch_, uint16_t _Width_us_)**
With:
	* **_Ch_**: the RC channel (1 to _ChNb_)
	* **_Width_us_**: the pulse width in µs

* **uint8_t TinyCppmGen.isSynchro()**:
	* CPPM Synchronization indicator: indicates that the pulse values have just been recorded for the current CPPM frame generation and gives 20 ms for preparing next pulse widths. This allows to pass digital information over CPPM (one different pulse width per CPPM frame). This is a "clear on read" function (no need to clear explicitely the indicator).

* **void TinyCppmGen.suspend()**:
	* Suspends the CPPM frame generation.

* **void TinyCppmGen.resume()**:
	* Resumes the CPPM frame generation.

Design considerations:
---------------------
As this library relies on Timer Output Compare capabilities, the CPPM output pin is imposed by the hardware and is target dependent.

However, there is some flexibility as the timer and the channel can be chosen by the user (in TinyCppmGen.h):

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

