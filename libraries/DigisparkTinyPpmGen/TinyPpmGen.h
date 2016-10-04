#ifndef TINY_PPM_GEN
#define TINY_PPM_GEN 1
/* A tiny interrupt driven RC PPM frame generator library using compare match of a 8 bits timer (timer used for ms in the arduino core can be reused)
   Features:
   - Uses Output Compare Channel A or B of the 8 bit Timer 0, 1 or 2. When used, it disables associated PWM -> Pin marked as "OCxy" shall be used as PPM Frame output (no other choice) 
   - Can generate a PPM Frame containing up to 12 RC Channels (8 channels 600 -> 2000 us with the 20ms default PPM period), up to 12 channels with higher PPM period.
   - Positive or Negative Modulation supported
   - Constant PPM Frame period: configurable from 10 to 40 ms (default = 20 ms)
   - No need to wait the PPM Frame period (usually 20 ms) to set the pulse width order for the channels, can be done at any time
   - Synchronisation indicator for digital data transmission over PPM
   - Blocking fonctions such as delay() can be used in the loop() since it's an interrupt driven PPM generator
   - Supported devices: (The user has to define Timer and Channel to use in TinyPpmGen.h file of the library)
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

RC Navy 2015
   http://p.loussouarn.free.fr
   31/01/2015: Creation
   14/02/2015: Timer and Channel choices added
   22/03/2015: Configurable PPM period in us added as optional argument in begin() method (default = 20ms)
   06/04/2015: Rcul support added (allows to create a virtual serial port over a PPM channel)
   09/11/2015: Bug in setChWidth_us() fixed and RAM size optimized (2 bytes per channel saved)
   31/01/2016: Support for ATmega32U4 (Arduino Leonardo, Micro and Pro Micro) added
   04/04/2016: Support for ATtiny84, suspend() and resume() methods added by Flavian Iliescu
*/
#include <Arduino.h>
#include <Rcul.h>

/* Constant definition: /!\ do NOT change this /!\ */
#define CH_A                           0xA /* /!\ Do NOT change this /!\ */
#define CH_B                           0xB /* /!\ Do NOT change this /!\ */

/************************************************************************/
/*                                                                      */
/*  FINAL USER: SELECT BELOW THE TIMER and THE CHANNEL YOU WANT TO USE  */
/*                                                                      */
/************************************************************************/
#define OC_TIMER      TIMER(0)    /* <-- Choose here the timer   between TIMER(0), TIMER(1) or TIMER(2) */
#define OC_CHANNEL    CHANNEL(A)  /* <-- Choose here the channel between CHANNEL(A) and CHANNEL(B) */

/*
ATtiny167 (Digispark pro):
=========================
TIMER(0), CHANNEL(A) -> OC0A -> PA2 -> Pin#8  Test: OK

ATtiny85 (Digispark):
====================
TIMER(0), CHANNEL(A) -> OC0A -> PB0 -> Pin#0  Test: OK
TIMER(0), CHANNEL(B) -> OC0B -> PB1 -> Pin#1  Test: OK
TIMER(1), CHANNEL(A) -> OC1A -> PB1 -> Pin#1  Test: OK
TIMER(1), CHANNEL(B) -> OC1B -> PB4 -> Pin#4  Test: Does NOT work for an unknown reason (Do NOT use it for now)


 ATtiny84 (Ext. Clock. 16MHz)
 =========================
 TIMER(0), CHANNEL(A) -> OC0A -> PB2 -> Pin#5 Test: OK
 TIMER(0), CHANNEL(B) -> OC0B -> PA7 -> Pin#6 Test: OK

ATmega328P (UNO):
================
TIMER(0), CHANNEL(A) -> OC0A -> PD6 -> Pin#6  Test: OK
TIMER(0), CHANNEL(B) -> OC0B -> PD5 -> Pin#5  Test: OK
TIMER(2), CHANNEL(A) -> OC2A -> PB3 -> Pin#11 Test: OK
TIMER(2), CHANNEL(B) -> OC2B -> PD3 -> Pin#3  Test: OK

ATmega32U4 (Arduino Leonardo, Micro and Pro Micro):
==================================================
TIMER(0), CHANNEL(A) -> OC0A -> PB7 -> Pin#11 Test: OK (/!\ pin not available on connector of Pro Micro /!\)
TIMER(0), CHANNEL(B) -> OC0B -> PD0 -> Pin#3  Test: OK

*/
/************************************************************************/
/*                                                                      */
/*  FINAL USER: SELECT ABOVE THE TIMER and THE CHANNEL YOU WANT TO USE  */
/*                                                                      */
/************************************************************************/


/* /!\ Do NOT change below /!\ */
#define TIMER(Timer)                   Timer
#define CHANNEL(Channel)               CH_##Channel

/* PPM Modulation choices: Positive or Negative */
#define TINY_PPM_GEN_POS_MOD           HIGH
#define TINY_PPM_GEN_NEG_MOD           LOW

#define DEFAULT_PPM_PERIOD             20000

/* Macro for PPM Gen client */
#define TINY_PPM_GEN_CLIENT(ClientIdx) (1 << (ClientIdx)) /* Range: 0 to 7 */

class OneTinyPpmGen : public Rcul
{
  private:
    // static data
    uint16_t _PpmPeriod_us;
  public:
    OneTinyPpmGen(void);
    uint8_t begin(uint8_t PpmModu, uint8_t ChNb, uint16_t PpmPeriod_us = DEFAULT_PPM_PERIOD);
    void    setChWidth_us(uint8_t Ch, uint16_t Width_us);
    uint8_t isSynchro(uint8_t SynchroClientMsk = TINY_PPM_GEN_CLIENT(7)); /* Default value: 8th Synchro client -> 0 to 6 free for other clients*/
    void    suspend(void);
    void    resume(void);
    /* Rcul support */
    virtual uint8_t  RculIsSynchro();
    virtual void     RculSetWidth_us(uint16_t Width_us, uint8_t Ch = 255);
    virtual uint16_t RculGetWidth_us(uint8_t Ch);
};

extern OneTinyPpmGen TinyPpmGen; /* Object externalisation */

#endif
