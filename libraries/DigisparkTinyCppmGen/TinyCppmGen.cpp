/* A tiny interrupt driven RC CPPM frame generator library using compare match of a 8 bits timer (timer used for ms in the arduino core can be reused)
   Features:
   - Uses Output Compare Channel A or B of the 8 bit Timer 0, 1 or 2. When used, it disables associated PWM -> Pin marked as "OCxy" shall be used as CPPM Frame output (no other choice) 
   - Can generate a CPPM Frame containing up to 12 RC Channels (8 channels 600 -> 2000 us with the 20ms default CPPM period), up to 12 channels with higher CPPM period.
   - Positive or Negative Modulation supported
   - Constant CPPM Frame period: configurable from 10 to 40 ms (default = 20 ms)
   - No need to wait the CPPM Frame period (usually 20 ms) to set the pulse width order for the channels, can be done at any time
   - Synchronisation indicator for digital data transmission over CPPM
   - Blocking fonctions such as delay() can be used in the loop() since it's an interrupt driven CPPM generator
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
   22/03/2015: Configurable CPPM period in us added as optional argument in begin() method (default = 20ms)
   06/04/2015: RcTxPop support added (allows to create a virtual serial port over a CPPM channel)
   09/11/2015: Bug in setChWidth_us() fixed and RAM size optimized (2 bytes per channel saved)
   31/01/2016: Support for ATmega32U4 (Arduino Leonardo, Micro and Pro Micro) added
   04/04/2016: Support for ATtiny84, suspend() and resume() methods added by Flavian Iliescu
*/
#include <TinyCppmGen.h>

/*********************************************************************************************/
/* Timer selection (0, 1 or 2) AND Channel selection (A or B) SHALL be done in TinyCppmGen.h */
/*********************************************************************************************/
/* /!\ Do NOT change below /!\ */
#if (OC_CHANNEL == CHANNEL(A))
#define OC_CHANNEL_LETTER          A
#else
#define OC_CHANNEL_LETTER          B
#endif

/* Macro for concatenation */
#define CONCAT2_(a, b)             a##b
#define CONCAT2(a, b)              CONCAT2_(a, b)

#define CONCAT3_(a, b, c)          a##b##c
#define CONCAT3(a, b, c)           CONCAT3_(a, b, c)

#define CONCAT4_(a, b, c, d)       a##b##c##d
#define CONCAT4(a, b, c, d)        CONCAT4_(a, b, c, d)

#define CONCAT5_(a, b, c, d, e)    a##b##c##d##e
#define CONCAT5(a, b, c, d, e)     CONCAT5_(a, b, c, d, e)


#ifndef MS_TIMER_TICK_EVERY_X_CYCLES /* SYMBOL introduced to the modified arduino distribution for Digispark */
#define MS_TIMER_TICK_EVERY_X_CYCLES 64 /* Default value for regular arduino distribution */
#endif

#if (MS_TIMER_TICK_EVERY_X_CYCLES < 16/*clockCyclesPerMicrosecond()*/)
#error MS_TIMER_TICK_EVERY_X_CYCLES too low!
#endif

#define MS_TIMER_TICK_DURATION_US   (MS_TIMER_TICK_EVERY_X_CYCLES / clockCyclesPerMicrosecond())

#if (F_CPU == 16500000)
#define CPPM_US_TO_TICK(Us)         (((Us) + ((Us) >> 5)) / MS_TIMER_TICK_DURATION_US) /* Correction for Digispark at 16.5 MHz */
#else
#define CPPM_US_TO_TICK(Us)         ((Us) / MS_TIMER_TICK_DURATION_US)
#endif

#ifdef __AVR_ATtiny167__
 #if ((OC_TIMER == TIMER(0)) && (OC_CHANNEL == CHANNEL(A)))
  #define CPPM_WF_REG                CONCAT3(TCCR,  OC_TIMER, A)
  #define CPPM_CS_REG                CONCAT3(TCCR,  OC_TIMER, B)
  #define CPPM_CM_REG                CONCAT3(TCCR,  OC_TIMER, A)
  #define CPPM_FORCE_REG             CONCAT3(TCCR,  OC_TIMER, B)
  #define TIM_MODE_NORMAL()          (CPPM_WF_REG &= ~(_BV(WGM01) | _BV(WGM00)));(CPPM_CS_REG = _BV(CS02))
  #define CPPM_OC_INT_MSK_REG        TIMSK0
  #define CPPM_PORT                  A
  #define PIN_BIT                    2
 #else
  #error TinyCppmGen SHALL use Timer0 and ChannelA for ATtiny167 !!!
 #endif

#else

#ifdef __AVR_ATtiny84__
 #if (OC_TIMER == TIMER(0))
  #define CPPM_WF_REG              CONCAT3(TCCR, OC_TIMER, A)
  #define CPPM_CS_REG              CONCAT3(TCCR, OC_TIMER, B)
  #define CPPM_CM_REG              CONCAT3(TCCR, OC_TIMER, A)
  #define CPPM_FORCE_REG           CONCAT3(TCCR, OC_TIMER, B)
  #define TIM_MODE_NORMAL()        (CPPM_WF_REG &= ~(_BV(WGM01) | _BV(WGM00))); (CPPM_CS_REG = (_BV(CS01) | _BV(CS00)))
  #define CPPM_OC_INT_MSK_REG      TIMSK0  
  #if (OC_CHANNEL == CHANNEL(A))
   #define CPPM_PORT B
   #define PIN_BIT   2
  #else
   #define CPPM_PORT A
   #define PIN_BIT   7
  #endif
 #else
  #error TinyCppmGen SHALL use Timer0 and ChannelA or ChannelB for ATtiny84  !!!
 #endif
     
#else
#ifdef  __AVR_ATtiny85__
#if (OC_TIMER == TIMER(0))
  #define CPPM_WF_REG              CONCAT3(TCCR,  OC_TIMER, A)
  #define CPPM_CS_REG              CONCAT3(TCCR,  OC_TIMER, B)
  #define CPPM_CM_REG              CONCAT3(TCCR,  OC_TIMER, A)
  #define CPPM_FORCE_REG           CONCAT3(TCCR,  OC_TIMER, B)
  #define TIM_MODE_NORMAL()        (CPPM_WF_REG &= ~(_BV(WGM01) | _BV(WGM00)));(CPPM_CS_REG = (_BV(CS01) | _BV(CS00)))
  #if (OC_CHANNEL == CHANNEL(A))
    #define PIN_BIT  0
  #else
    #define PIN_BIT  1
  #endif
#else
  #if (OC_TIMER == TIMER(1))
    /* No PPM_WF_REG needed */
    #define CPPM_CS_REG            CONCAT2(TCCR,  OC_TIMER)
    #define CPPM_FORCE_REG         GTCCR
    #define TIM_MODE_NORMAL()      (CPPM_CS_REG = (_BV(CS12) | _BV(CS11) | _BV(CS10)))
    #if (OC_CHANNEL == CHANNEL(A))
      #define PIN_BIT  1
      #define CPPM_CM_REG          CONCAT2(TCCR,  OC_TIMER)
    #else
      #define PIN_BIT  4
      #define CPPM_CM_REG          GTCCR
    #endif
  #else
    #error TinyCppmGen supports only 8 bit Timers (Timer0 and Timer1 on ATtiny85) !!!  
  #endif
#endif
#define CPPM_OC_INT_MSK_REG        TIMSK
#define CPPM_PORT                  B
#else
#ifdef __AVR_ATmega328P__
#define CPPM_WF_REG                CONCAT3(TCCR,  OC_TIMER, A)
#define CPPM_CS_REG                CONCAT3(TCCR,  OC_TIMER, B)
#define CPPM_CM_REG                CONCAT3(TCCR,  OC_TIMER, A)
#define CPPM_FORCE_REG             CONCAT3(TCCR,  OC_TIMER, B)
#if (OC_TIMER == TIMER(0))
  #define CPPM_PORT                D
  #define CPPM_OC_INT_MSK_REG      TIMSK0
  #define TIM_MODE_NORMAL()        (CPPM_WF_REG &= ~(_BV(CONCAT3(WGM,OC_TIMER,1)) | _BV(CONCAT3(WGM,OC_TIMER,0))));(CPPM_CS_REG = (_BV(CONCAT3(CS,OC_TIMER,1)) | _BV(CONCAT3(CS,OC_TIMER,0))))
  #if (OC_CHANNEL == CHANNEL(A))
    #define PIN_BIT  6
  #else
    #define PIN_BIT  5
  #endif
#else
  #if (OC_TIMER == TIMER(2))
    #define CPPM_OC_INT_MSK_REG    TIMSK2
    #define TIM_MODE_NORMAL()      (CPPM_WF_REG &= ~(_BV(CONCAT3(WGM,OC_TIMER,1)) | _BV(CONCAT3(WGM,OC_TIMER,0))));(CPPM_CS_REG = _BV(CONCAT3(CS,OC_TIMER,2)))
    #define PIN_BIT                3
    #if (OC_CHANNEL == CHANNEL(A))
      #define CPPM_PORT            B
    #else
      #define CPPM_PORT            D
    #endif
  #else
    #error TinyCppmGen supports UNO only with 8 bit Timers (Timer0 and Timer2) !!!
  #endif
#endif
#else
#ifdef __AVR_ATmega32U4__
#if (OC_TIMER == TIMER(0))
  #define CPPM_WF_REG              CONCAT3(TCCR,  OC_TIMER, A)
  #define CPPM_CS_REG              CONCAT3(TCCR,  OC_TIMER, B)
  #define CPPM_CM_REG              CONCAT3(TCCR,  OC_TIMER, A)
  #define CPPM_FORCE_REG           CONCAT3(TCCR,  OC_TIMER, B)
  #define TIM_MODE_NORMAL()        (CPPM_WF_REG &= ~(_BV(WGM01) | _BV(WGM00)));(CPPM_CS_REG = (_BV(CS01) | _BV(CS00)))
  #define CPPM_OC_INT_MSK_REG      TIMSK0
  #if (OC_CHANNEL == CHANNEL(A))
    #define CPPM_PORT B
    #define CPIN_BIT  7
#warning CHANNEL(A)
  #else
    #define CPPM_PORT D
    #define CPIN_BIT  0
#warning CHANNEL(B)
  #endif
#else
  #error TinyCppmGen SHALL use Timer0 for ATmega32U4 !!!
#endif
#else
#error This target is not supported (yet) by the TinyCppmGen library!!!
#endif
#endif
#endif
#endif
#endif

#define CPPM_OC_PORT               CONCAT2(PORT,CPPM_PORT)
#define CPPM_OC_DDR                CONCAT2(DDR, CPPM_PORT)
#define CPPM_OC_PIN                CONCAT2(PIN, CPPM_PORT)
#define CPPM_OC_PIN_MSK            (1 << PIN_BIT)

#ifdef __AVR_ATtiny84__
 #define COMP_VECT                  CONCAT5(TIM,   OC_TIMER, _COMP, OC_CHANNEL_LETTER, _vect)
#else
 #define COMP_VECT                  CONCAT5(TIMER, OC_TIMER, _COMP, OC_CHANNEL_LETTER, _vect)
#endif

#define CPPM_OCR                   CONCAT3(OCR,   OC_TIMER, OC_CHANNEL_LETTER)
#define OCIE_MASK                  _BV(CONCAT3(OCIE, OC_TIMER, OC_CHANNEL_LETTER))
#define OC_FORCE_MASK              _BV(CONCAT3(FOC,  OC_TIMER, OC_CHANNEL_LETTER))

#define CPPM_OC_INT_ENABLE()       CPPM_OC_INT_MSK_REG |=  OCIE_MASK
#define CPPM_OC_INT_DISABLE()      CPPM_OC_INT_MSK_REG &= ~OCIE_MASK

#define CPPM_OC_FORCE()            CPPM_FORCE_REG |= OC_FORCE_MASK

#define TOGGLE_CPPM_PIN_DISABLE()  (CPPM_CM_REG &= ~(_BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 1)) | _BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 0))))
#define TOGGLE_CPPM_PIN_ENABLE()   (CPPM_CM_REG |=   _BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 0)))

#define INIT_CPPM_PIN(PpmModu)     do{\
                                    if(CppmModu == TINY_CPPM_GEN_NEG_MOD)\
                                    {\
                                      CPPM_OC_PIN |= CPPM_OC_PIN_MSK; /* Set pin to high */\
                                      CPPM_CM_REG |= (_BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 1)) | _BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 0)));\
                                    }\
                                    else\
                                    {\
                                      CPPM_OC_PIN &= ~CPPM_OC_PIN_MSK; /* Set pin to low */\
                                      CPPM_CM_REG |= (_BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 1)));CPPM_CM_REG &= ~(_BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 0)));\
                                    delay(1);\
                                    }\
                                    CPPM_OC_FORCE();\
                                    TCNT0=0;\
                                  }while(0)

#define FULL_OVF_MASK             0x7F
#define HALF_OVF_MASK             0x80
#define HALF_OVF_VAL              128

#define CHANNEL_MAX_NB            12

#define SYNCHRO_MIN_US            3500
#define CHANNEL_MAX_US            2000

#define CPPM_HEADER_US            300

#define CPPM_NEUTRAL_US           1500

#define CPPM_FRAME_MIN_PERIOD_US  10000
#define CPPM_FRAME_MAX_PERIOD_US  40000

#define CPPM_GUARD_TICK           16 /* Time to be sure to have time to prepare remaing ticks */

/*
Positive CPPM   .-----.                         .-----.
                |     |                         |     |
                |  P  |  F     F     F    H   R |  P  |
                |     |                         |     |
             ---'     '-----x-----x-----x---x---'     '-----
                <------------------------------->
                        Channel Duration
                        
Negative CPPM---.     .-----x-----x-----x---x---.     .-----
                |     |                         |     |
                |  P  |  F     F     F    H   R |  P  |
                |     |                         |     |
                '-----'                         '-----'
                <------------------------------->
                        Channel Duration
Legend:
======
 P: Pulse header duration in ticks (Nb of Ticks corresponding to CPPM_HEADER_US us)
 F: Full timer overflow (256 Ticks)
 H: Half timer overflow (128 Ticks)
 R: Remaining Ticks to complete the full Channel duration
*/

typedef struct {
  uint8_t Ovf;
  uint8_t Rem;
}OneChSt_t;

/* Global variables */
static volatile OneChSt_t* _Next = NULL;
static volatile OneChSt_t  _Cur;
static volatile uint8_t    _Synchro = 0;
static volatile uint8_t    _StartOfPulse = 1;
static volatile uint8_t    _Idx = 0;
static volatile uint8_t    _ChMaxNb;

OneTinyCppmGen TinyCppmGen = OneTinyCppmGen();


/* Public functions */
OneTinyCppmGen::OneTinyCppmGen(void) /* Constructor */
{
  
}

uint8_t OneTinyCppmGen::begin(uint8_t CppmModu, uint8_t ChNb, uint16_t CppmPeriod_us /*= DEFAULT_PPM_PERIOD*/)
{
  uint8_t Ok = false;
  uint8_t Ch;

  _ChMaxNb = (ChNb > CHANNEL_MAX_NB)?CHANNEL_MAX_NB:ChNb; /* Max is 12 Channels */
  _Next    = (OneChSt_t *)malloc(sizeof(OneChSt_t) * (_ChMaxNb + 1)); /* + 1 for Synchro Channel */
  Ok = (_Next != NULL);
  if (Ok)
  {
    _Idx = _ChMaxNb; /* To reload values at startup */
    _CppmPeriod_us = CppmPeriod_us;
    if(_CppmPeriod_us < CPPM_FRAME_MIN_PERIOD_US)                                 _CppmPeriod_us = CPPM_FRAME_MIN_PERIOD_US;
    if(_CppmPeriod_us < ((_ChMaxNb * (uint16_t)CHANNEL_MAX_US) + SYNCHRO_MIN_US)) _CppmPeriod_us = (_ChMaxNb * (uint16_t)CHANNEL_MAX_US) + SYNCHRO_MIN_US;
    if(_CppmPeriod_us > CPPM_FRAME_MAX_PERIOD_US)                                  _CppmPeriod_us = CPPM_FRAME_MAX_PERIOD_US;
    for(Ch = 1; Ch <= _ChMaxNb; Ch++) /* Init all the channels to Neutral */
    {
      setChWidth_us(Ch, CPPM_NEUTRAL_US);
    }
#if 1
#warning TO DO: fix issue with positive modulation ! (bad signal randomly generated)
    /* Set Pin as Output according to the PPM modulation level */
    CPPM_OC_DDR |= CPPM_OC_PIN_MSK; /* Set pin as output */
    if(CppmModu == TINY_CPPM_GEN_NEG_MOD)
    {
      CPPM_OC_PIN |= CPPM_OC_PIN_MSK; /* Set pin to high */
    }
    else
    {
      CPPM_OC_PIN &= ~CPPM_OC_PIN_MSK; /* Set pin to low */
    }
    TIM_MODE_NORMAL();
    if(CppmModu == TINY_CPPM_GEN_POS_MOD)
    {
      CPPM_OC_FORCE(); /* Force Output Compare to initialize properly the output */
    }
    else delay(1);
#else
    //test
    CPPM_OC_DDR |= CPPM_OC_PIN_MSK; /* Set pin as output */
    TIM_MODE_NORMAL();
    INIT_CPPM_PIN(PpmModu);
#endif
    TOGGLE_CPPM_PIN_ENABLE();
    CPPM_OC_INT_ENABLE();
  }
  return(Ok);
}

void OneTinyCppmGen::setChWidth_us(uint8_t Ch, uint16_t Width_us)
{
  uint16_t ChTickNb, SumTick = 0, SynchTick;
  uint8_t  Ch_Next_Ovf, Ch_Next_Rem, Ch0_Next_Ovf, Ch0_Next_Rem;

  if((Ch >= 1) && (Ch <= _ChMaxNb))
  {
    ChTickNb = CPPM_US_TO_TICK(Width_us - CPPM_HEADER_US + (MS_TIMER_TICK_DURATION_US / 2)); /* Convert in rounded Timer Ticks */
    Ch_Next_Ovf = (ChTickNb & 0xFF00) >> 8;
    Ch_Next_Rem = (ChTickNb & 0x00FF);
    if(Ch_Next_Rem < CPPM_GUARD_TICK)
    {
      Ch_Next_Ovf |= HALF_OVF_MASK;
      Ch_Next_Rem += HALF_OVF_VAL;
    }
    /* Update Synchro Time */
    for(uint8_t Idx = 1; Idx <= _ChMaxNb; Idx++)
    {
      SumTick += CPPM_US_TO_TICK(CPPM_HEADER_US);
      if(Idx != Ch)
      {
        SumTick += ((_Next[Idx].Ovf & FULL_OVF_MASK) << 8) + _Next[Idx].Rem;
        if(_Next[Idx].Ovf & HALF_OVF_MASK) SumTick -= HALF_OVF_VAL;
      }
      else
      {
        SumTick += ((Ch_Next_Ovf & FULL_OVF_MASK) << 8) + Ch_Next_Rem;
        if(Ch_Next_Ovf & HALF_OVF_MASK) SumTick -= HALF_OVF_VAL;
      }
    }
    SynchTick = CPPM_US_TO_TICK(_CppmPeriod_us) - SumTick - CPPM_US_TO_TICK(CPPM_HEADER_US);
    Ch0_Next_Ovf = (SynchTick & 0xFF00) >> 8;
    Ch0_Next_Rem = (SynchTick & 0x00FF);
    if(Ch0_Next_Rem < CPPM_GUARD_TICK)
    {
      Ch0_Next_Ovf |= HALF_OVF_MASK;
      Ch0_Next_Rem += HALF_OVF_VAL;
    }
    /* Update requested Channel AND Synchro to keep constant the period (20ms) */
    CPPM_OC_INT_DISABLE();
    _Next[0].Ovf  = Ch0_Next_Ovf;
    _Next[0].Rem  = Ch0_Next_Rem;
    _Next[Ch].Ovf = Ch_Next_Ovf;
    _Next[Ch].Rem = Ch_Next_Rem;
    CPPM_OC_INT_ENABLE();
  }
}

uint8_t OneTinyCppmGen::isSynchro(uint8_t ClientIdx /*= 7*/)
{
  uint8_t Ret;
  
  Ret = !!(_Synchro & RCUL_CLIENT_MASK(ClientIdx));
  if(Ret) _Synchro &= ~RCUL_CLIENT_MASK(ClientIdx); /* Clear indicator for the Synchro client */
  
  return(Ret);
}

void OneTinyCppmGen::suspend(void)
{
  CPPM_OC_INT_DISABLE();
  TOGGLE_CPPM_PIN_DISABLE();
}

void OneTinyCppmGen::resume(void)
{
  TOGGLE_CPPM_PIN_ENABLE();
  CPPM_OC_INT_ENABLE();
}

/* Begin of Rcul support */
uint8_t OneTinyCppmGen::RculIsSynchro(uint8_t ClientIdx /*= RCUL_DEFAULT_CLIENT_IDX*/)
{
  return(isSynchro(ClientIdx));
}

void OneTinyCppmGen::RculSetWidth_us(uint16_t Width_us, uint8_t Ch /*= RCUL_NO_CH*/)
{
  setChWidth_us(Ch, Width_us); /* Take care about argument order (like that, since Ch will be optional for other pulse generator) */
}

uint16_t OneTinyCppmGen::RculGetWidth_us(uint8_t Ch)
{
  Ch = Ch; /* To avoid a compilation warning */
  return(0);
}

/* End of Rcul support */

SIGNAL(COMP_VECT)
{
  if(_StartOfPulse)
  {
     /* Modify PPM_OCR only if Tick > 1 us */
#if (MS_TIMER_TICK_DURATION_US > 1)
    CPPM_OCR += CPPM_US_TO_TICK(CPPM_HEADER_US);
#endif
   /* Next Channel or Synchro */
    _Idx++;
    if(_Idx > _ChMaxNb)
    {
      /* End of PPM Frame */
      _Idx = 0;
      _Synchro = 0xFF; /* OK: Widths loaded */
    }
    /* Generate Next Channel or Synchro */
    _Cur.Ovf = _Next[_Idx].Ovf;
    _Cur.Rem = _Next[_Idx].Rem;
    _StartOfPulse = 0;
  }
  else
  {
    /* Do not change PPM_OCR to have a full Ovf */
    if(_Cur.Rem)
    {
      CPPM_OCR += _Cur.Rem;  /* Remain to generate */
      _Cur.Rem = 0;
    }
    else
    {
      if(_Cur.Ovf)
      {
        if(_Cur.Ovf & HALF_OVF_MASK)
        {
          _Cur.Ovf &= FULL_OVF_MASK; /* Clear Half ovf indicator */
          CPPM_OCR += HALF_OVF_VAL; /* Half Overflow to generate */
        }
        _Cur.Ovf--;
      }
    }
    if(!_Cur.Ovf && !_Cur.Rem)
    {
      TOGGLE_CPPM_PIN_ENABLE();
      _StartOfPulse = 1;
    }
    else
    {
      TOGGLE_CPPM_PIN_DISABLE();      
    }
  }
}
