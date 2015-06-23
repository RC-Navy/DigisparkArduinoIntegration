/* A tiny interrupt driven RC PPM frame generator library using compare match of a 8 bits timer (timer used for ms in the arduino core can be reused)
   Features:
   - Uses Output Compare Channel A or B of the 8 bit Timer 0, 1 or 2. When used, it disables associated PWM -> Pin marked as "OCxy" shall be used as PPM Frame output (no other choice) 
   - Can generate a PPM Frame containing up to 8 RC Channels (600 -> 2000 us) or 7 RC Channels (600 -> 2400 us) with default PPM period (20ms), 8 with (higher PPM period)
   - Positive or Negative Modulation supported
   - Constant PPM Frame period: configurable from 10 to 40 ms (default = 20 ms)
   - No need to wait 20 ms to set the pulse width order for the channels, can be done at any time
   - Synchronisation indicator for digital data transmission over PPM
   - Blocking fonctions such as delay() can be used in the loop() since it's an interrupt driven PPM generator
   - Supported devices: (The user has to define Timer and Channel to use in TinyPpmGen.h file of the library)
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
         
   RC Navy 2015
   http://p.loussouarn.free.fr
   31/01/2015: Creation
   14/02/2015: Timer and Channel choices added
   22/03/2015: Configurable PPM period in us added as optional argument in begin() method (default = 20ms)
   06/04/2015: RcTxPop support added (allows to create a virtual serial port over a PPM channel)
*/
#include <TinyPpmGen.h>

/********************************************************************************************/
/* Timer selection (0, 1 or 2) AND Channel selection (A or B) SHALL be done in TinyPpmGen.h */
/********************************************************************************************/
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

#define MS_TIMER_TICK_DURATION_US  (MS_TIMER_TICK_EVERY_X_CYCLES / clockCyclesPerMicrosecond())

#if (F_CPU == 16500000)
#define PPM_US_TO_TICK(Us)         (((Us) + ((Us) >> 5)) / MS_TIMER_TICK_DURATION_US) /* Correction for Digispark at 16.5 MHz */
#else
#define PPM_US_TO_TICK(Us)         ((Us) / MS_TIMER_TICK_DURATION_US)
#endif

#ifdef __AVR_ATtiny167__
#if ((OC_TIMER == TIMER(0)) && (OC_CHANNEL == CHANNEL(A)))
#define PPM_WF_REG                 CONCAT3(TCCR,  OC_TIMER, A)
#define PPM_CS_REG                 CONCAT3(TCCR,  OC_TIMER, B)
#define PPM_CM_REG                 CONCAT3(TCCR,  OC_TIMER, A)
#define PPM_FORCE_REG              CONCAT3(TCCR,  OC_TIMER, B)
#define TIM_MODE_NORMAL()          (PPM_WF_REG &= ~(_BV(WGM01) | _BV(WGM00)));(PPM_CS_REG = _BV(CS02))
#define PPM_OC_INT_MSK_REG         TIMSK0
#define PPM_PORT                   A
#define PIN_BIT                    2
#else
#error TinyPpmGen SHALL use Timer0 and ChannelA for ATtiny167 !!!
#endif
#else
#ifdef  __AVR_ATtiny85__
#if (OC_TIMER == TIMER(0))
  #define PPM_WF_REG               CONCAT3(TCCR,  OC_TIMER, A)
  #define PPM_CS_REG               CONCAT3(TCCR,  OC_TIMER, B)
  #define PPM_CM_REG               CONCAT3(TCCR,  OC_TIMER, A)
  #define PPM_FORCE_REG            CONCAT3(TCCR,  OC_TIMER, B)
  #define TIM_MODE_NORMAL()        (PPM_WF_REG &= ~(_BV(WGM01) | _BV(WGM00)));(PPM_CS_REG = (_BV(CS01) | _BV(CS00)))
  #if (OC_CHANNEL == CHANNEL(A))
    #define PIN_BIT  0
  #else
    #define PIN_BIT  1
  #endif
#else
  #if (OC_TIMER == TIMER(1))
    /* No PPM_WF_REG needed */
    #define PPM_CS_REG             CONCAT2(TCCR,  OC_TIMER)
    #define PPM_FORCE_REG          GTCCR
    #define TIM_MODE_NORMAL()      (PPM_CS_REG = (_BV(CS12) | _BV(CS11) | _BV(CS10)))
    #if (OC_CHANNEL == CHANNEL(A))
      #define PIN_BIT  1
      #define PPM_CM_REG           CONCAT2(TCCR,  OC_TIMER)
    #else
      #define PIN_BIT  4
      #define PPM_CM_REG           GTCCR
    #endif
  #else
    #error TinyPpmGen supports only 8 bit Timers (Timer0 and Timer1 on ATtiny85) !!!  
  #endif
#endif
#define PPM_OC_INT_MSK_REG         TIMSK
#define PPM_PORT                   B
#else
#ifdef __AVR_ATmega328P__
#define PPM_WF_REG                 CONCAT3(TCCR,  OC_TIMER, A)
#define PPM_CS_REG                 CONCAT3(TCCR,  OC_TIMER, B)
#define PPM_CM_REG                 CONCAT3(TCCR,  OC_TIMER, A)
#define PPM_FORCE_REG              CONCAT3(TCCR,  OC_TIMER, B)
#if (OC_TIMER == TIMER(0))
  #define PPM_PORT                 D
  #define PPM_OC_INT_MSK_REG       TIMSK0
  #define TIM_MODE_NORMAL()        (PPM_WF_REG &= ~(_BV(CONCAT3(WGM,OC_TIMER,1)) | _BV(CONCAT3(WGM,OC_TIMER,0))));(PPM_CS_REG = (_BV(CONCAT3(CS,OC_TIMER,1)) | _BV(CONCAT3(CS,OC_TIMER,0))))
  #if (OC_CHANNEL == CHANNEL(A))
    #define PIN_BIT  6
  #else
    #define PIN_BIT  5
  #endif
#else
  #if (OC_TIMER == TIMER(2))
    #define PPM_OC_INT_MSK_REG     TIMSK2
    #define TIM_MODE_NORMAL()      (PPM_WF_REG &= ~(_BV(CONCAT3(WGM,OC_TIMER,1)) | _BV(CONCAT3(WGM,OC_TIMER,0))));(PPM_CS_REG = _BV(CONCAT3(CS,OC_TIMER,2)))
    #define PIN_BIT                3
    #if (OC_CHANNEL == CHANNEL(A))
      #define PPM_PORT             B
    #else
      #define PPM_PORT             D
    #endif
  #else
    #error TinyPpmGen supports UNO only with 8 bit Timers (Timer0 and Timer2) !!!
  #endif
#endif
#else
#error This target is not supported (yet) by the TinyPpmGn library!!!
#endif
#endif
#endif

#define PPM_OC_PORT                CONCAT2(PORT,PPM_PORT)
#define PPM_OC_DDR                 CONCAT2(DDR, PPM_PORT)
#define PPM_OC_PIN                 CONCAT2(PIN, PPM_PORT)
#define PPM_OC_PIN_MSK             (1 << PIN_BIT)

#define COMP_VECT                  CONCAT5(TIMER, OC_TIMER, _COMP, OC_CHANNEL_LETTER, _vect)
#define PPM_OCR                    CONCAT3(OCR,   OC_TIMER, OC_CHANNEL_LETTER)
#define OCIE_MASK                  _BV(CONCAT3(OCIE, OC_TIMER, OC_CHANNEL_LETTER))
#define OC_FORCE_MASK              _BV(CONCAT3(FOC,  OC_TIMER, OC_CHANNEL_LETTER))

#define PPM_OC_INT_ENABLE()        PPM_OC_INT_MSK_REG |=  OCIE_MASK
#define PPM_OC_INT_DISABLE()       PPM_OC_INT_MSK_REG &= ~OCIE_MASK

#define PPM_OC_FORCE()             PPM_FORCE_REG |= OC_FORCE_MASK

#define TOGGLE_PPM_PIN_DISABLE()  (PPM_CM_REG &= ~(_BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 1)) | _BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 0))))
#define TOGGLE_PPM_PIN_ENABLE()   (PPM_CM_REG |=   _BV(CONCAT4(COM, OC_TIMER, OC_CHANNEL_LETTER, 0)))


#define FULL_OVF_MASK             0x7F
#define HALF_OVF_MASK             0x80
#define HALF_OVF_VAL              128

#define PPM_HEADER_US             300

#define PPM_NEUTRAL_US            1500

#define PPM_FRAME_MIN_PERIOD_US   10000
#define PPM_FRAME_MAX_PERIOD_US   40000

#define PPM_GUARD_TICK            16 /* Time to be sure to have time to prepare remaing ticks */

/*
Positive PPM    .-----.                         .-----.
                |     |                         |     |
                |  P  |  F     F     F    H   R |  P  |
                |     |                         |     |
             ---'     '-----x-----x-----x---x---'     '-----
                <------------------------------->
                        Channel Duration
                        
Negative PPM ---.     .-----x-----x-----x---x---.     .-----
                |     |                         |     |
                |  P  |  F     F     F    H   R |  P  |
                |     |                         |     |
                '-----'                         '-----'
                <------------------------------->
                        Channel Duration
Legend:
======
 P: Pulse header duration in ticks (Nb of Ticks corresponding to PPM_HEADER_US us)
 F: Full timer overflow (256 Ticks)
 H: Half timer overflow (128 Ticks)
 R: Remaining Ticks to complete the full Channel duration
*/

typedef struct {
  uint8_t Ovf;
  uint8_t Rem;
}OneChSt_t;

typedef struct {
  OneChSt_t Cur;
  OneChSt_t Next;
}ChSt_t;

/* Global variables */
static volatile ChSt_t*   _Ch = NULL;
static volatile uint8_t   _Synchro = 0;
static volatile uint8_t   _StartOfFrame = 1;
static volatile uint8_t   _Idx = 0;
static volatile uint8_t   _ChMaxNb;

OneTinyPpmGen TinyPpmGen = OneTinyPpmGen();


/* Public functions */
OneTinyPpmGen::OneTinyPpmGen(void) /* Constructor */
{
  
}

uint8_t OneTinyPpmGen::begin(uint8_t PpmModu, uint8_t ChNb, uint16_t PpmPeriod_us /*= DEFAULT_PPM_PERIOD*/)
{
  boolean Ok = false;
  uint8_t Ch;

  _ChMaxNb = (ChNb > 8)?8:ChNb; /* Max is 8 Channels */
  _Ch = (ChSt_t *)malloc(sizeof(ChSt_t) * (_ChMaxNb + 1)); /* + 1 for Synchro Channel */
  Ok = (_Ch != NULL);
  if (Ok)
  {
    _Idx = _ChMaxNb; /* To reload values at startup */
    _PpmPeriod_us = PpmPeriod_us;
    if(_PpmPeriod_us < PPM_FRAME_MIN_PERIOD_US) _PpmPeriod_us = PPM_FRAME_MIN_PERIOD_US;
    if(_PpmPeriod_us > PPM_FRAME_MAX_PERIOD_US) _PpmPeriod_us = PPM_FRAME_MAX_PERIOD_US;
    for(Ch = 1; Ch <= _ChMaxNb; Ch++)
    {
      setChWidth_us(Ch, PPM_NEUTRAL_US);
    }
    /* Set Pin as Output according to the PPM modulation level */
    PPM_OC_DDR |= PPM_OC_PIN_MSK; /* Set pin as output */
    if(PpmModu == TINY_PPM_GEN_NEG_MOD)
    {
      PPM_OC_PIN |= PPM_OC_PIN_MSK; /* Set pin to high */
    }
    else
    {
      PPM_OC_PIN &= ~PPM_OC_PIN_MSK; /* Set pin to low */
    }
    TIM_MODE_NORMAL();
    if(PpmModu == TINY_PPM_GEN_POS_MOD)
    {
      PPM_OC_FORCE(); /* Force Output Compare to initialize properly the output */
    }
    else delay(1);
    TOGGLE_PPM_PIN_ENABLE();
    PPM_OC_INT_ENABLE();
  }
  return(Ok);
}

void OneTinyPpmGen::setChWidth_us(uint8_t Ch, uint16_t Width_us)
{
  uint16_t ChTickNb, SumTick = 0, SynchTick;
  uint8_t  Ch_Next_Ovf, Ch_Next_Rem, Ch0_Next_Ovf, Ch0_Next_Rem;

  if((Ch >= 1) && (Ch <= _ChMaxNb))
  {
    ChTickNb = PPM_US_TO_TICK(Width_us - PPM_HEADER_US + (MS_TIMER_TICK_DURATION_US / 2)); /* Convert in rounded Timer Ticks */
    Ch_Next_Ovf = (ChTickNb & 0xFF00) >> 8;
    Ch_Next_Rem = (ChTickNb & 0x00FF);
    if(Ch_Next_Rem < PPM_GUARD_TICK)
    {
      Ch_Next_Ovf |= HALF_OVF_MASK;
      Ch_Next_Rem += HALF_OVF_VAL;
    }
    /* Update Synchro Time */
    for(uint8_t Idx = 1; Idx <= _ChMaxNb; Idx++)
    {
      SumTick += PPM_US_TO_TICK(PPM_HEADER_US);
      if(Idx != Ch)
      {
        SumTick += ((_Ch[Idx].Cur.Ovf & FULL_OVF_MASK) << 8) + _Ch[Idx].Next.Rem;
        if(_Ch[Idx].Cur.Ovf & HALF_OVF_MASK) SumTick -= HALF_OVF_VAL;
      }
      else
      {
        SumTick += ((Ch_Next_Ovf & FULL_OVF_MASK) << 8) + Ch_Next_Rem;
        if(Ch_Next_Ovf & HALF_OVF_MASK) SumTick -= HALF_OVF_VAL;
      }
    }
    SynchTick = PPM_US_TO_TICK(_PpmPeriod_us) - SumTick - PPM_US_TO_TICK(PPM_HEADER_US);
    Ch0_Next_Ovf = (SynchTick & 0xFF00) >> 8;
    Ch0_Next_Rem = (SynchTick & 0x00FF);
    if(Ch0_Next_Rem < PPM_GUARD_TICK)
    {
      Ch0_Next_Ovf |= HALF_OVF_MASK;
      Ch0_Next_Rem += HALF_OVF_VAL;
    }
    /* Update requested Channel AND Synchro to keep constant the period (20ms) */
    PPM_OC_INT_DISABLE();
    _Ch[0].Next.Ovf = Ch0_Next_Ovf;
    _Ch[0].Next.Rem = Ch0_Next_Rem;
    _Ch[Ch].Next.Ovf = Ch_Next_Ovf;
    _Ch[Ch].Next.Rem = Ch_Next_Rem;
    PPM_OC_INT_ENABLE();
  }
}

uint8_t OneTinyPpmGen::isSynchro(uint8_t SynchroClientMsk /*= TINY_PPM_GEN_CLIENT(7)*/)
{
  uint8_t Ret;
  
  Ret = !!(_Synchro & SynchroClientMsk);
  if(Ret) _Synchro &= ~SynchroClientMsk; /* Clear indicator for the Synchro client */
  
  return(Ret);
}
/* Begin of RcTxPop support */
uint8_t OneTinyPpmGen::RcTxPopIsSynchro()
{
  return(isSynchro(TINY_PPM_GEN_CLIENT(6)));
}

void OneTinyPpmGen::RcTxPopSetWidth_us(uint16_t Width_us, uint8_t Ch /*= 255*/)
{
  setChWidth_us(Ch, Width_us); /* Take care about argument order (like that, since Ch will be optional for other pulse generator) */
}
/* End of RcTxPop support */

SIGNAL(COMP_VECT)
{
  if(_StartOfFrame)
  {
     /* Modify PPM_OCR only if Tick > 1 us */
#if (MS_TIMER_TICK_DURATION_US > 1)
    PPM_OCR += PPM_US_TO_TICK(PPM_HEADER_US);
#endif
   /* Next Channel or Synchro */
    _Idx++;
    if(_Idx > _ChMaxNb)
    {
      /* End of PPM Frame */
      _Idx = 0;
      /* Reload new Channel values including Synchro */
      for(uint8_t Idx = 0; Idx <= _ChMaxNb; Idx++)
      {
        _Ch[Idx].Cur.Ovf = _Ch[Idx].Next.Ovf;
        _Ch[Idx].Cur.Rem = _Ch[Idx].Next.Rem;
      }
      _Synchro = 0xFF; /* OK: Widths loaded */
    }
    /* Generate Next Channel or Synchro */
    _Ch[0].Cur.Ovf = _Ch[_Idx].Cur.Ovf;
    _Ch[0].Cur.Rem = _Ch[_Idx].Cur.Rem;
    _StartOfFrame = 0;
  }
  else
  {
    /* Do not change PPM_OCR to have a full Ovf */
    if(_Ch[0].Cur.Rem)
    {
      PPM_OCR += _Ch[0].Cur.Rem;  /* Remain to generate */
      _Ch[0].Cur.Rem = 0;
    }
    else
    {
      if(_Ch[0].Cur.Ovf)
      {
        if(_Ch[0].Cur.Ovf & HALF_OVF_MASK)
        {
          _Ch[0].Cur.Ovf &= FULL_OVF_MASK; /* Clear Half ovf indicator */
          PPM_OCR += HALF_OVF_VAL; /* Half Overflow to generate */
        }
        _Ch[0].Cur.Ovf--;
      }
    }
    if(!_Ch[0].Cur.Ovf && !_Ch[0].Cur.Rem)
    {
      TOGGLE_PPM_PIN_ENABLE();
      _StartOfFrame = 1;
    }
    else
    {
      TOGGLE_PPM_PIN_DISABLE();      
    }
  }
}
