/* A tiny interrupt driven RC CPPM frame reader library using pin change interrupt
   Features:
   - Uses any input supporting interrupt pin change
   - Supported devices: see TinyPinChange library
   - Positive and negative CPPM modulation supported (don't care)
   - Up to 9 RC channels supported
   RC Navy 2015
   http://p.loussouarn.free.fr
   01/02/2015: Creation
   06/04/2015: Rcul support added (allows to create a virtual serial port over a CPPM channel)
   09/11/2015: No need to create the TinyCppmReader object anymore, unused _PinMask variable removed
   23/03/2020: Multi instance support added (Now, the object(s) shall be created)
*/
#include <TinyCppmReader.h>


/*
EXAMPLE OF POSITIVE AND NEGATIVE CPPM FRAME TRANSPORTING 2 RC CHANNELS
======================================================================

 Positive CPPM
       .-----.                 .-----.         .-----.                                  .-----.                 .-----.         .-----. 
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
       |     |                 |     |         |     |                                  |     |                 |     |         |     |
    ---'     '-----------------'     '---------'     '----------------//----------------'     '-----------------'     '---------'     '----
       <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
               Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2
             <-----------------------><--------------><---------------------//---------------><-----------------------><-------------->
                     Channel#1           Channel#2                       Synchro                       Channel#1           Channel#2
                        
 Negative CPPM
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

static uint8_t  _CppmFrameInputPin;
static int8_t   _VirtualPort;
static volatile uint8_t  _Synchro;
static volatile uint16_t _ChWidthUs[TINY_CPPM_READER_CH_MAX];
static volatile uint8_t  _ChIdx;
static volatile uint8_t  _ChIdxMax;
static volatile uint16_t _PrevEdgeUs;
static volatile uint16_t _StartCppmPeriodUs;
static volatile uint16_t _CppmPeriodUs;

/* Public functions */
TinyCppmReader::TinyCppmReader(void) /* Constructor */
{
}

uint8_t TinyCppmReader::attach(uint8_t CppmInputPin)
{
  uint8_t Ret = 0;
  
  TinyPinChange_Init();
  _VirtualPort = TinyPinChange_RegisterIsr(CppmInputPin, TinyCppmReader::rcChannelCollectorIsr);
  if(_VirtualPort >= 0)
  {
    for(uint8_t Idx = 0; Idx < TINY_CPPM_READER_CH_MAX; Idx++)
    {
      _ChWidthUs[Idx] = NEUTRAL_US;
    }
    _ChIdx    = (TINY_CPPM_READER_CH_MAX + 1);
    _ChIdxMax = 0;
    _Synchro  = 0;
    _CppmFrameInputPin = CppmInputPin;
    TinyPinChange_EnablePin(_CppmFrameInputPin);
    Ret = 1;
  }
  return(Ret);
}

uint8_t TinyCppmReader::detach(void)
{
  suspend();
  return(TinyPinChange_UnregisterIsr(_CppmFrameInputPin, TinyCppmReader::rcChannelCollectorIsr));
}

uint8_t TinyCppmReader::detectedChannelNb(void)
{ 
  return(_ChIdxMax); /* No need to mask/unmask interrupt (8 bits) */
}

uint16_t TinyCppmReader::width_us(uint8_t Ch)
{
  uint16_t Width_us = 1500;
  if((Ch >= 1) && (Ch <= TinyCppmReader::detectedChannelNb()))
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

uint16_t TinyCppmReader::cppmPeriod_us(void)
{
  uint16_t CppmPeriod_us = 0;
#if 1
    /* Read CPPM Period without disabling interrupts */
    do
    {
      CppmPeriod_us = _CppmPeriodUs;
    }while(CppmPeriod_us != _CppmPeriodUs);
#else
  cli();
  CppmPeriod_us = _CppmPeriodUs;
  sei();
#endif
  return(CppmPeriod_us);
}

uint8_t TinyCppmReader::isSynchro(uint8_t ClientIdx /*= 7*/)
{
  uint8_t Ret;
  
  Ret = !!(_Synchro & RCUL_CLIENT_MASK(ClientIdx));
  if(Ret) _Synchro &= ~RCUL_CLIENT_MASK(ClientIdx); /* Clear indicator for the Synchro client */
  
  return(Ret);
}

/* Begin of Rcul support */
uint8_t TinyCppmReader::RculIsSynchro(uint8_t ClientIdx /*= RCUL_DEFAULT_CLIENT_IDX*/)
{
  return(isSynchro(ClientIdx));
}

uint16_t TinyCppmReader::RculGetWidth_us(uint8_t Ch)
{
  return(width_us(Ch));
}

void TinyCppmReader::RculSetWidth_us(uint16_t Width_us, uint8_t Ch /*= RCUL_NO_CH*/)
{
  Width_us = Width_us; /* To avoid a compilation warning */
  Ch = Ch;             /* To avoid a compilation warning */
}

/* End of Rcul support */

void TinyCppmReader::suspend(void)
{
  TinyPinChange_DisablePin(_CppmFrameInputPin);
  _ChIdx = (TINY_CPPM_READER_CH_MAX + 1);
}

void TinyCppmReader::resume(void)
{
  _PrevEdgeUs = (uint16_t)(micros() & 0xFFFF);
  TinyPinChange_EnablePin(_CppmFrameInputPin);
}

/* ISR */
void TinyCppmReader::rcChannelCollectorIsr(void)
{
  static uint8_t Period = false;
  uint16_t CurrentEdgeUs, PulseDurationUs;
  
  if(TinyPinChange_RisingEdge(_VirtualPort, _CppmFrameInputPin))
  {
    CurrentEdgeUs   = (uint16_t)(micros() & 0xFFFF);
    PulseDurationUs = CurrentEdgeUs - _PrevEdgeUs;
    _PrevEdgeUs = CurrentEdgeUs;
    if(PulseDurationUs >= SYNCHRO_TIME_MIN_US)
    {
      _ChIdxMax = _ChIdx;
      _ChIdx    = 0;
      _Synchro  = 0xFF; /* Synchro detected */
      Period = !Period;
      if(Period) _StartCppmPeriodUs = CurrentEdgeUs;
      else       _CppmPeriodUs      = CurrentEdgeUs - _StartCppmPeriodUs;
    }
    else
    {
      if(_ChIdx < TINY_CPPM_READER_CH_MAX)
      {
        _ChWidthUs[_ChIdx] = PulseDurationUs;
        _ChIdx++;
      }
    }
  }

}
