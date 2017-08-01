/*
 English: by RC Navy (2012-2015)
 =======
 <SoftRcPulseIn>: an asynchronous library to read Input Pulse Width from standard Hobby Radio-Control. This library is a non-blocking version of pulseIn().
 http://p.loussouarn.free.fr
 V1.0: initial release
 V1.1: asynchronous timeout support added (up to 250ms)
 V1.2: (06/04/2015) RcTxPop and RcRxPop support added (allows to create a virtual serial port over a PPM channel)
 V1.3: (12/04/2016) boolean type replaced by uint8_t and version management replaced by constants
 V1.4: (04/10/2016) Update with Rcul in replacement of RcTxPop and RcRxPop
 V1.5: (31/06/2017) Support for Arduino ESP8266 added, support of inverted pulse addded
 
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
*/

#include "SoftRcPulseIn.h"


SoftRcPulseIn *SoftRcPulseIn::last = NULL;

#ifdef ESP8266
static uint16_t PinsImage = 0xFFFF;
#endif

#ifdef SOFT_RC_PULSE_IN_INV_SUPPORT
SoftRcPulseIn::SoftRcPulseIn(uint8_t Inv /*= 0*/)
{
  _Inv = Inv;
}
#else
SoftRcPulseIn::SoftRcPulseIn(void)
{
}
#endif

uint8_t SoftRcPulseIn::attach(uint8_t Pin, uint16_t PulseMin_us/*=600*/, uint16_t PulseMax_us/*=2400*/)
{
  uint8_t Ret = 0;

#ifdef ESP8266
  if(Pin < 16)
  {
    _Pin     = Pin;
    _Min_us  = PulseMin_us;
    _Max_us  = PulseMax_us;
    prev  = last;
    last = this;
    pinMode(_Pin, INPUT);
    digitalWrite(_Pin, HIGH); /* Eanble Pull-up */
    attachInterrupt(digitalPinToInterrupt(_Pin), SoftRcPulseIn::SoftRcPulseInInterrupt, CHANGE);
    Ret = 1;
  }
#else
  _VirtualPortIdx = TinyPinChange_RegisterIsr(Pin, SoftRcPulseIn::SoftRcPulseInInterrupt);
  if(_VirtualPortIdx >= 0)
  {
    _Pin     = Pin;
    _PinMask = TinyPinChange_PinToMsk(Pin);
    _Min_us  = PulseMin_us;
    _Max_us  = PulseMax_us;
    prev  = last;
    last = this;
    pinMode(_Pin, INPUT);
    digitalWrite(_Pin, HIGH); /* Eanble Pull-up */
    TinyPinChange_EnablePin(_Pin);
    Ret = 1;
  }
#endif
  return(Ret);
}

uint8_t SoftRcPulseIn::available(void)
{
  uint8_t  Ret = 0;
  uint16_t PulseWidth_us;

  if(_Available)
  {
	noInterrupts();
	PulseWidth_us = _Width_us;
	interrupts();
	Ret = (PulseWidth_us >= _Min_us) && (PulseWidth_us <= _Max_us);
	_Available = 0;
  }
  return(Ret);
}

#ifdef SOFT_RC_PULSE_IN_TIMEOUT_SUPPORT
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
#endif

uint16_t SoftRcPulseIn::width_us(void)
{
  uint16_t PulseWidth_us;

  noInterrupts();
  PulseWidth_us = _Width_us;
  interrupts();
  
  return(PulseWidth_us);  
}

/* Begin of Rcul support */
uint8_t SoftRcPulseIn::RculIsSynchro()
{
  return(available());
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

void SoftRcPulseIn::SoftRcPulseInInterrupt(void)
{
  SoftRcPulseIn *RcPulseIn;
  uint8_t        PinState;

  for(RcPulseIn = last; RcPulseIn != 0; RcPulseIn = RcPulseIn->prev)
  {
#ifdef ESP8266
    PinState = digitalRead(RcPulseIn->_Pin);
    if((PinsImage & (1 << RcPulseIn->_Pin)) ^ (PinState << RcPulseIn->_Pin))
    {
#else
    if(TinyPinChange_Edge(RcPulseIn->_VirtualPortIdx, RcPulseIn->_Pin))
    {
      PinState = digitalRead(RcPulseIn->_Pin);
#endif
	  if(PinState
#ifdef SOFT_RC_PULSE_IN_INV_SUPPORT
                  ^ RcPulseIn->_Inv
#endif        
      )
	  {
		  /* High level, rising edge: start chrono */
		  RcPulseIn->_Start_us = micros();
	  }
	  else
	  {
		  /* Low level, falling edge: stop chrono */
		  RcPulseIn->_Width_us = micros() - RcPulseIn->_Start_us;
		  RcPulseIn->_Available = 1;
#ifdef SOFT_RC_PULSE_IN_TIMEOUT_SUPPORT
		  RcPulseIn->_LastTimeStampMs = (uint8_t)(millis() & 0x000000FF);
#endif
	  }
#ifdef ESP8266
      bitWrite(PinsImage, RcPulseIn->_Pin, PinState); /* Update real pin state in image */
#endif
    }
  }
}
