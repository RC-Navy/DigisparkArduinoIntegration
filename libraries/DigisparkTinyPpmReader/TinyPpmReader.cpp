/* A tiny interrupt driven RC PPM frame reader library using pin change interrupt
   Features:
   - Uses any input supporting interrupt pin change
   - Supported devices: see TinyPinChange library
   - Positive and negative PPM modulation supported (don't care)
   - Up to 9 RC channels supported
   RC Navy 2015
   http://p.loussouarn.free.fr
   01/02/2015: Creation
   06/04/2015: Rcul support added (allows to create a virtual serial port over a PPM channel)
   09/11/2015: No need to create the TinyPpmReader object anymore, unused _PinMask variable removed
*/
#include <TinyPpmReader.h>


/*
EXAMPLE OF POSITIVE AND NEGATIVE PPM FRAME TRANSPORTING 2 RC CHANNELS
=====================================================================

 Positive PPM
       .-----.                 .-----.         .-----.                                  .-----.                 .-----.         .-----. 
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
    ---'     '-----------------'     '---------'     '----------------//----------------'     '-----------------'     '---------'     '----
       <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
               Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2
             <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
                     Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2
                        
 Negative PPM
    ---.     .-----------------.     .---------.     .----------------//----------------.     .-----------------.     .---------'     .----       
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       '-----'                 '-----'         '-----'                                  '-----'                 '-----'         '-----' 
       <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
               Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2
             <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
                     Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2

 The channel durations (Pulse width) are located between 2 rising edges.
 Please, note the same channel durations (Pulse width) are obtained between 2 falling edges.
 The Synchro pulse shall be longer than the longer RC Channel pulse width.
*/

#define NEUTRAL_US           1500
#define SYNCHRO_TIME_MIN_US  3000

TinyPpmReaderClass TinyPpmReader = TinyPpmReaderClass();

/* Public functions */
TinyPpmReaderClass::TinyPpmReaderClass(void) /* Constructor */
{
}

uint8_t TinyPpmReaderClass::attach(uint8_t PpmInputPin)
{
  uint8_t Ret = 0;
  
  TinyPinChange_Init();
  _VirtualPort = TinyPinChange_RegisterIsr(PpmInputPin, TinyPpmReaderClass::rcChannelCollectorIsr);
  if(_VirtualPort >= 0)
  {
    for(uint8_t Idx = 0; Idx < TINY_PPM_READER_CH_MAX; Idx++)
    {
      _ChWidthUs[Idx] = NEUTRAL_US;
    }
    _ChIdx    = (TINY_PPM_READER_CH_MAX + 1);
    _ChIdxMax = 0;
    _Synchro  = 0;
    _PpmFrameInputPin = PpmInputPin;
    TinyPinChange_EnablePin(_PpmFrameInputPin);
    Ret = 1;
  }
  return(Ret);
}

uint8_t TinyPpmReaderClass::detectedChannelNb(void)
{ 
  return(_ChIdxMax); /* No need to mask/unmask interrupt (8 bits) */
}

uint16_t TinyPpmReaderClass::width_us(uint8_t Ch)
{
  uint16_t Width_us = 1500;
  if((Ch >= 1) && (Ch <= TinyPpmReaderClass::detectedChannelNb()))
  {
    Ch--;
#if 1
    /* Read pulse width without disabling interrupts */
    do
    {
      Width_us = _ChWidthUs[Ch];
    }while(Width_us != _ChWidthUs[Ch]);
#else
    cli();
    Width_us = _ChWidthUs[Ch];
    sei();
#endif
  }
  return(Width_us);
}

uint16_t TinyPpmReaderClass::ppmPeriod_us(void)
{
  uint16_t PpmPeriod_us = 0;
#if 1
    /* Read PPM Period without disabling interrupts */
    do
    {
      PpmPeriod_us = _PpmPeriodUs;
    }while(PpmPeriod_us != _PpmPeriodUs);
#else
  cli();
  PpmPeriod_us = _PpmPeriodUs;
  sei();
#endif
  return(PpmPeriod_us);
}

uint8_t TinyPpmReaderClass::isSynchro(uint8_t SynchroClientMsk /*= TINY_PPM_READER_CLIENT(7)*/)
{
  uint8_t Ret;
  
  Ret = !!(_Synchro & SynchroClientMsk);
  if(Ret) _Synchro &= ~SynchroClientMsk; /* Clear indicator for the Synchro client */
  
  return(Ret);
}

/* Begin of Rcul support */
uint8_t TinyPpmReaderClass::RculIsSynchro()
{
  return(isSynchro(TINY_PPM_READER_CLIENT(6)));
}

uint16_t TinyPpmReaderClass::RculGetWidth_us(uint8_t Ch)
{
  return(width_us(Ch));
}

void     TinyPpmReaderClass::RculSetWidth_us(uint16_t Width_us, uint8_t Ch /*= 255*/)
{
  Width_us = Width_us; /* To avoid a compilation warning */
  Ch = Ch;             /* To avoid a compilation warning */
}

/* End of Rcul support */

void TinyPpmReaderClass::suspend(void)
{
  TinyPinChange_DisablePin(_PpmFrameInputPin);
  _ChIdx = (TINY_PPM_READER_CH_MAX + 1);
}

void TinyPpmReaderClass::resume(void)
{
  _PrevEdgeUs = (uint16_t)(micros() & 0xFFFF);
  TinyPinChange_EnablePin(_PpmFrameInputPin);
}

/* ISR */
void TinyPpmReaderClass::rcChannelCollectorIsr(void)
{
  static uint8_t Period = false;
  TinyPpmReaderClass *PpmReader = &TinyPpmReader;
  uint16_t CurrentEdgeUs, PulseDurationUs;
  
  if(TinyPinChange_RisingEdge(PpmReader->_VirtualPort, PpmReader->_PpmFrameInputPin))
  {
    CurrentEdgeUs   = (uint16_t)(micros() & 0xFFFF);
    PulseDurationUs = CurrentEdgeUs - PpmReader->_PrevEdgeUs;
    PpmReader->_PrevEdgeUs = CurrentEdgeUs;
    if(PulseDurationUs >= SYNCHRO_TIME_MIN_US)
    {
      PpmReader->_ChIdxMax = PpmReader->_ChIdx;
      PpmReader->_ChIdx    = 0;
      PpmReader->_Synchro  = 0xFF; /* Synchro detected */
      Period = !Period;
      if(Period) PpmReader->_StartPpmPeriodUs = CurrentEdgeUs;
      else       PpmReader->_PpmPeriodUs      = CurrentEdgeUs - PpmReader->_StartPpmPeriodUs;
    }
    else
    {
      if(PpmReader->_ChIdx < TINY_PPM_READER_CH_MAX)
      {
	PpmReader->_ChWidthUs[PpmReader->_ChIdx] = PulseDurationUs;
	PpmReader->_ChIdx++;
      }
    }
  }

}
