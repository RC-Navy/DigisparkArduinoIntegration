/*
RcPpmChSubst sketch
by RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 2015
This sketch reads an RC PPM frame, extracts the numbers of channels and their pulse witdhs and substitutes the 4th channel with a fix pulse width (2000 us).
This sketch can work with a Digispark pro, Digispark and Arduino UNO.
The PPM input shall support pin change interrupt.

PPM output pin is imposed by hardware and is target dependant:
(The user has to define Timer and Channel to use in TinyPpmGen.h file of the library)
       - ATtiny167 (Digispark pro):
         TIMER(0), CHANNEL(A) -> OC0A -> PA2 -> Pin#8

       - ATtiny85 (Digispark):
         TIMER(0), CHANNEL(A) -> OC0A -> PB0 -> Pin#0
         TIMER(0), CHANNEL(B) -> OC0B -> PB1 -> Pin#1
         TIMER(1), CHANNEL(A) -> OC1A -> PB1 -> Pin#1
         
       - ATmega328P (Arduino UNO):
         TIMER(0), CHANNEL(A) -> OC0A -> PD6 -> Pin#6
         TIMER(0), CHANNEL(B) -> OC0B -> PD5 -> Pin#5
         TIMER(2), CHANNEL(A) -> OC2A -> PB3 -> Pin#11
         TIMER(2), CHANNEL(B) -> OC2B -> PD3 -> Pin#3
This example code is in the public domain.
*/
#include <TinyPinChange.h>
#include <TinyPpmReader.h>
#include <TinyPpmGen.h>

#define PPM_INPUT_PIN  2

TinyPpmReader MyPpmReader;  /* Create a TinyPpmReader object */

void setup()
{
  TinyPpmGen.begin(TINY_PPM_GEN_POS_MOD, 4); /* Change TINY_PPM_GEN_POS_MOD to TINY_PPM_GEN_NEG_MOD for NEGative PPM modulation */
  MyPpmReader.attach(PPM_INPUT_PIN); /* Attach MyPpmReader to PPM_INPUT_PIN pin */
}

void loop()
{
  if((MyPpmReader.detectedChannelNb() >= 4) && MyPpmReader.isSynchro())
  {
    TinyPpmGen.setChWidth_us(1, MyPpmReader.width_us(1)); /* RC Channel#1: forward rx value */
    TinyPpmGen.setChWidth_us(2, MyPpmReader.width_us(2)); /* RC Channel#2: forward rx value */
    TinyPpmGen.setChWidth_us(3, MyPpmReader.width_us(3)); /* RC Channel#3 forward rx value: */
    TinyPpmGen.setChWidth_us(4, 2000); /* RC Channel#4: replace rx pulse width with 2000 us */
  }
}
