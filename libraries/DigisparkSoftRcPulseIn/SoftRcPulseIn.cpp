/*
 English: by RC Navy (2012-2020)
 =======
 <SoftRcPulseIn>: an asynchronous library to read Input Pulse Width from standard Hobby Radio-Control. This library is a non-blocking version of pulseIn().
 http://p.loussouarn.free.fr
 V1.0: initial release
 V1.1: asynchronous timeout support added (up to 250ms)
 V1.2: (06/04/2015) RcTxPop and RcRxPop support added (allows to create a virtual serial port over a PPM channel)
 V1.3: (12/04/2016) boolean type replaced by uint8_t and version management replaced by constants
 V1.4: (04/10/2016) Update with Rcul in replacement of RcTxPop and RcRxPop
 V1.5: (31/06/2017) Support for Arduino ESP8266 added, support of inverted pulse addded
 V1.6: (16/06/2018) Support for multi "Synchro" client addded
 V1.7: (20/04/2020) Support of virtual port distinction to avoid side effects when pins are declared in diffrent ports
 Francais: par RC Navy (2012-2015)
 ========
 <SoftRcPulseIn>: une librairie asynchrone pour lire les largeur d'impulsions des Radio-Commandes standards. Cette librairie est une version non bloquante de pulseIn().
 http://p.loussouarn.free.fr
 V1.0: release initiale
 V1.1: support de timeout asynchrone ajoutee (jusqu'a 250ms)
 V1.2: (06/04/2015) Support de RcTxPop et RcRxPop ajoute (permet de creer un port serie virtuel par dessus une voie PPM)
 V1.3: (12/04/2016) type boolean remplace par uint8_t et gestion de version remplace par des constantes
 V1.4: (04/10/2016) Mise a jour avec Rcul en remplacement de RcTxPop et RcRxPop
 V1.5: (31/06/2017) Ajout du support de l'Arduino ESP8266, ajout du support des impulsions inversees
 V1.6: (16/06/2018) Ajout du support pour plusieurs clients de "Synchro"
 V1.7: (20/04/2020) Ajout distinction de Virtual Ports pour eviter des effets de bord quand des pins sont declares dans des ports differents
*/

#include "SoftRcPulseIn.h"


SoftRcPulseIn *SoftRcPulseIn::last = NULL;

#ifdef ESP8266
static uint16_t PinsImage = 0xFFFF;
#endif

SoftRcPulseIn::SoftRcPulseIn(uint8_t Inv /*= 0*/)
{
  _Info.Inv = Inv;
}

int8_t SoftRcPulseIn::attach(uint8_t Pin, uint16_t PulseMin_us/*=600*/, uint16_t PulseMax_us/*=2400*/)
{
  int8_t Ret = -1;

#ifdef ESP8266
  if(Pin < 16)
  {
    _Pin     = Pin;
    _Min_us  = PulseMin_us;
    _Max_us  = PulseMax_us;
    _Info.VirtualPortIdx = 0;
    prev  = last;
    last = this;
    pinMode(_Pin, INPUT);
    digitalWrite(_Pin, HIGH); /* Eanble Pull-up */
    attachInterrupt(digitalPinToInterrupt(_Pin), SoftRcPulseIn::SoftRcPulseInInterrupt0, CHANGE);
    Ret = 1;
  }
#else
  _Info.VirtualPortIdx = DigitalPinToPortIdx(Pin);
  switch(_Info.VirtualPortIdx)
  {
    case 0:
    _Info.VirtualPortIdx = TinyPinChange_RegisterIsr(Pin, SoftRcPulseIn::SoftRcPulseInInterrupt0ISR);
    break;

#if defined(__AVR_ATtinyX4__) || defined(__AVR_ATtiny167__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__)   
    case 1:
    _Info.VirtualPortIdx = TinyPinChange_RegisterIsr(Pin, SoftRcPulseIn::SoftRcPulseInInterrupt1ISR);
    break;
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
    case 2:
    _Info.VirtualPortIdx = TinyPinChange_RegisterIsr(Pin, SoftRcPulseIn::SoftRcPulseInInterrupt2ISR);
    break;
#endif
  }
  if(_Info.VirtualPortIdx >= 0)
  {
    _Pin     = Pin;
    _Min_us  = PulseMin_us;
    _Max_us  = PulseMax_us;
    prev  = last;
    last = this;
    pinMode(_Pin, INPUT);
    digitalWrite(_Pin, HIGH); /* Eanble Pull-up */
    TinyPinChange_EnablePin(_Pin);
    Ret = _Info.VirtualPortIdx;
  }
#endif
  return(Ret);
}

void SoftRcPulseIn::detach(void)
{
  TinyPinChange_DisablePin(_Pin);
}

uint8_t SoftRcPulseIn::available(uint8_t ClientIdx /*= 7*/)
{
  uint8_t  Ret = 0;
  uint16_t PulseWidth_us;

  if(isSynchro(ClientIdx))
  {
    /* Read Pulse without disabling interrupts */
    do
    {
      PulseWidth_us = _Width_us;
    }while(PulseWidth_us != _Width_us);

    Ret = (PulseWidth_us >= _Min_us) && (PulseWidth_us <= _Max_us);
  }
  return(Ret);
}

uint8_t SoftRcPulseIn::isSynchro(uint8_t ClientIdx /*= 7*/)
{
  uint8_t Ret;
  
  Ret = !!(_Available & RCUL_CLIENT_MASK(ClientIdx));
  if(Ret) _Available &= ~RCUL_CLIENT_MASK(ClientIdx); /* Clear indicator for the Synchro client */
  
  return(Ret);
}

uint8_t SoftRcPulseIn::timeout(uint8_t TimeoutMs, uint8_t *CurrentState)
{
  uint8_t CurMs, Ret = 0;

  CurMs = (uint8_t)(millis() & 0x000000FF);
  if((uint8_t)(CurMs - _LastTimeStampMs) >= TimeoutMs)
  {
    *CurrentState = digitalRead(_Pin);
    Ret = 1;
  }
  return(Ret);
}

uint16_t SoftRcPulseIn::width_us(void)
{
  uint16_t PulseWidth_us;

  /* Read Pulse without disabling interrupts */
  do
  {
    PulseWidth_us = _Width_us;
  }while(PulseWidth_us != _Width_us);

  return(PulseWidth_us);  
}

uint32_t SoftRcPulseIn::start_us(void)
{
  uint32_t Start_us;
  
  /* Read Pulse start without disabling interrupts */
  do
  {
    Start_us = _Start_us;
  }while(Start_us != _Start_us);

  return(Start_us);
}

/* Begin of Rcul support */
uint8_t SoftRcPulseIn::RculIsSynchro(uint8_t ClientIdx /*= RCUL_DEFAULT_CLIENT_IDX*/)
{
  return(available(ClientIdx));
}

uint16_t SoftRcPulseIn::RculGetWidth_us(uint8_t Ch)
{
  Ch = Ch; /* To avoid a compilation warning */
  return(width_us());
}

void     SoftRcPulseIn::RculSetWidth_us(uint16_t Width_us, uint8_t Ch /*= 255*/)
{
  Width_us = Width_us; /* To avoid a compilation warning */
  Ch = Ch;             /* To avoid a compilation warning */
}
/* End of Rcul support */

#define DECLARE_SOFT_RC_PULSE_IN_ISR(VirtualPort)                                           \
void SoftRcPulseIn::SoftRcPulseInInterrupt##VirtualPort##ISR(void)                          \
{                                                                                           \
  SoftRcPulseIn *RcPulseIn;                                                                 \
  uint8_t        PinState;                                                                  \
  uint32_t       NowUs;                                                                     \
                                                                                            \
  NowUs = micros();                                                                         \
  for(RcPulseIn = last; RcPulseIn != 0; RcPulseIn = RcPulseIn->prev)                        \
  {                                                                                         \
    if(RcPulseIn->_Info.VirtualPortIdx != VirtualPort) continue;                            \
    if(TinyPinChange_Edge(RcPulseIn->_Info.VirtualPortIdx, RcPulseIn->_Pin))                \
    {                                                                                       \
      PinState = digitalRead(RcPulseIn->_Pin);                                              \
      if(PinState  ^ RcPulseIn->_Info.Inv)                                                  \
      {                                                                                     \
        /* High level, rising edge: start chrono */                                         \
        RcPulseIn->_Start_us = NowUs;                                                       \
      }                                                                                     \
      else                                                                                  \
      {                                                                                     \
        /* Low level, falling edge: stop chrono */                                          \
        RcPulseIn->_Width_us = (uint16_t)(NowUs - RcPulseIn->_Start_us);                    \
        RcPulseIn->_Available = 0xFF;                                                       \
        RcPulseIn->_LastTimeStampMs = (uint8_t)(millis() & 0x000000FF);                     \
      }                                                                                     \
    }                                                                                       \
  }                                                                                         \
}

DECLARE_SOFT_RC_PULSE_IN_ISR(0)

#if defined(__AVR_ATtinyX4__) || defined(__AVR_ATtiny167__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__)
DECLARE_SOFT_RC_PULSE_IN_ISR(1)
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
DECLARE_SOFT_RC_PULSE_IN_ISR(2)
#endif

