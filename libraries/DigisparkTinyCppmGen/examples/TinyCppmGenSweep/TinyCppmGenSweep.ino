/*
TinyCppmGenSweep sketch
by RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 2015
This sketch generates an RC CPPM frame transporting 4 RC channels.
The 3 first channels have fixed pulse width (tunable), and the 4th channel sweeps between 1000 and 2000 us.
It can be extended up to 8 RC channels.
This sketch can work with a Digispark pro, Digispark, Arduino Leonardo, Arduino Micro, Arduino Pro Micro and Arduino UNO.

PPM output pin is imposed by hardware and is target dependant:
(The user has to define Timer and Channel to use in TinyCppmGen.h file of the library)

       - ATtiny167 (Digispark pro):
         TIMER(0), CHANNEL(A) -> OC0A -> PA2 -> Pin#8

       - ATtiny85 (Digispark):
         TIMER(0), CHANNEL(A) -> OC0A -> PB0 -> Pin#0
         TIMER(0), CHANNEL(B) -> OC0B -> PB1 -> Pin#1
         TIMER(1), CHANNEL(A) -> OC1A -> PB1 -> Pin#1
         
       - ATtiny84 (Ext. Clock. 16MHz) -> Fuses: LF:0xFE, HF:0xDF, EF: 0xFF 
         TIMER(0), CHANNEL(A) -> OC0A -> PB2 -> Pin#5 | Digital 8 : D8
         TIMER(0), CHANNEL(B) -> OC0B -> PA7 -> Pin#6 | Digital 7 : D7

         - ATmega328P (Arduino UNO):
         TIMER(0), CHANNEL(A) -> OC0A -> PD6 -> Pin#6
         TIMER(0), CHANNEL(B) -> OC0B -> PD5 -> Pin#5
         TIMER(2), CHANNEL(A) -> OC2A -> PB3 -> Pin#11
         TIMER(2), CHANNEL(B) -> OC2B -> PD3 -> Pin#3

         - ATmega32U4 (Arduino Leonardo, Micro and Pro Micro):
         TIMER(0), CHANNEL(A) -> OC0A -> PB7 -> Pin#11 (/!\ pin not available on connector of Pro Micro /!\)
         TIMER(0), CHANNEL(B) -> OC0B -> PD0 -> Pin#3

This example code is in the public domain.
*/
#include <TinyCppmGen.h>
#include <Rcul.h>

#define CH_MAX_NB  4

#define STEP_US    5

#define PULSE_WIDTH_MIN_US    1000
#define PULSE_WIDTH_MAX_US    2000
#define CPPM_PERIOD_US        20000

uint16_t Width_us = PULSE_WIDTH_MAX_US;
uint16_t Step_us  = STEP_US;

void setup()
{
  TinyCppmGen.begin(TINY_CPPM_GEN_POS_MOD, CH_MAX_NB, CPPM_PERIOD_US); /* Change CTINY_PPM_GEN_POS_MOD to TINY_CPPM_GEN_NEG_MOD for NEGative CPPM modulation */
  TinyCppmGen.setChWidth_us(1, 500);  /* RC Channel#1 */
  TinyCppmGen.setChWidth_us(2, 1000); /* RC Channel#2 */
  TinyCppmGen.setChWidth_us(3, 1500); /* RC Channel#3 */
  TinyCppmGen.setChWidth_us(4, 2000); /* RC Channel#4 */
}

void loop()
{
  TinyCppmGen.setChWidth_us(CH_MAX_NB, Width_us); /* Sweep RC Channel#4 */
  Width_us += Step_us;
  if(Width_us > PULSE_WIDTH_MAX_US) Step_us = -STEP_US;
  if(Width_us < PULSE_WIDTH_MIN_US) Step_us = +STEP_US;
  delay(10);
}
