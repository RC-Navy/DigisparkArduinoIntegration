#include "Arduino.h"
#include <RcRxSerial.h>
#include <avr/pgmspace.h>
/*
 English: by RC Navy (2012)
 =======
 <RcTxSerial>: a library to build an unidirectionnal serial port through RC Transmitter/Receiver.
 http://p.loussouarn.free.fr

 Francais: par RC Navy (2012)
 ========
 <RcTxSerial>: une librairie pour construire un port serie a travers un Emetteur/Recepteur RC.
 http://p.loussouarn.free.fr
*/

/*
  STEP
  <->
 992                                                                 2080
  |-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|
    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   I
    <--->
    |   |                                                           |
  1024 1088                                                        2048
INTER_NIBBLE
*/
enum {NIBBLE_0=0, NIBBLE_1, NIBBLE_2, NIBBLE_3, NIBBLE_4, NIBBLE_5, NIBBLE_6, NIBBLE_7, NIBBLE_8, NIBBLE_9, NIBBLE_A, NIBBLE_B, NIBBLE_C, NIBBLE_D, NIBBLE_E, NIBBLE_F, NIBBLE_I, NIBBLE_NB};

#define STEP                      	  32 /* 32 */

#define PULSE_MIN_VAL (1024 - 32) /* Not 1000:  992 allows 64us width for each nibble */
#define PULSE_MAX_VAL (2048 + 32) /* Not 2000: 2080 allows 64us width for each nibble */

#define INTER_NIBBLE			  (2*STEP) /* 64 */
#define PULSE_MIN(NibbleIdx)             (PULSE_MIN_VAL + ((NibbleIdx) * INTER_NIBBLE))
#define PULSE_MAX(NibbleIdx)             (PULSE_MIN((NibbleIdx)) + INTER_NIBBLE)
#define PULSE_MIN_MAX(NibbleIdx)         {PULSE_MIN(NibbleIdx), PULSE_MAX(NibbleIdx)}

typedef struct {
  boolean Available;
  boolean Nibble;
  char    Char;
}RcMsgRxSt_t;

typedef struct {
  uint16_t Min;
  uint16_t Max;
}NibblePulseSt_t;

const NibblePulseSt_t NibblePulse[NIBBLE_NB] PROGMEM = {
							  PULSE_MIN_MAX(NIBBLE_0),
							  PULSE_MIN_MAX(NIBBLE_1),
							  PULSE_MIN_MAX(NIBBLE_2),
							  PULSE_MIN_MAX(NIBBLE_3),
							  PULSE_MIN_MAX(NIBBLE_4),
							  PULSE_MIN_MAX(NIBBLE_5),
							  PULSE_MIN_MAX(NIBBLE_6),
							  PULSE_MIN_MAX(NIBBLE_7),
							  PULSE_MIN_MAX(NIBBLE_8),
							  PULSE_MIN_MAX(NIBBLE_9),
							  PULSE_MIN_MAX(NIBBLE_A),
							  PULSE_MIN_MAX(NIBBLE_B),
							  PULSE_MIN_MAX(NIBBLE_C),
							  PULSE_MIN_MAX(NIBBLE_D),
							  PULSE_MIN_MAX(NIBBLE_E),
							  PULSE_MIN_MAX(NIBBLE_F),
							  PULSE_MIN_MAX(NIBBLE_I)
							  };

#define TABLE_ITEM_NBR(Tbl)  (sizeof(Tbl) / sizeof(Tbl[0]))
                                  
static int8_t PulseWithToNibbleIdx(uint16_t PulseWidth);

/*************************************************************************
                              GLOBAL VARIABLES
*************************************************************************/
/* Constructor */
RcRxSerial::RcRxSerial(Rcul *Rcul, uint8_t Ch /*= 255*/)
{
	reassignRculSrc(Rcul, Ch);
}

void RcRxSerial::reassignRculSrc(Rcul *Rcul, uint8_t Ch /*= 255*/)
{
	_Ch = Ch;
	_Rcul = Rcul;
	_available = 0;
	_Nibble = 0;
	_NibbleAvailable = 0;
	_Char = 0;
	_MsgLen = 0;
}

#define RX_SERIAL_IDLE_NIBBLE_AVAILABLE  (1<< 0)
#define RX_SERIAL_CHAR_AVAILABLE         (1<< 1)
uint8_t RcRxSerial::somethingAvailable(void)
{
  uint8_t Ret = 0;
  uint8_t NibbleIdx;

  if(_Rcul->RculIsSynchro())
  {
     _LastWidth_us = _Rcul->RculGetWidth_us(_Ch);
     _NibbleAvailable = 1;
     NibbleIdx = PulseWithToNibbleIdx(_LastWidth_us);
     if(NibbleIdx >= 0)
     {
       if(NibbleIdx == NIBBLE_I)
       {
         Ret = RX_SERIAL_IDLE_NIBBLE_AVAILABLE;
         _Nibble = 0; /* Idle -> Re-Synch */
       }
       else
       {
         if(!_Nibble)
         {
            _Char = (NibbleIdx << 4);  /* MSN first */
         }
         else
         {
            _Char |= NibbleIdx;  /* LSN */
            Ret = RX_SERIAL_CHAR_AVAILABLE;
            _available = 1;
         }
         _Nibble = !_Nibble;
       }
     }
     else
     {
       _Nibble = 0; /* Unknown -> Re-Synch */
     }
  }
  return(Ret);
}

uint8_t RcRxSerial::available()
{
  somethingAvailable();
  return(_available);
}

uint8_t RcRxSerial::msgAvailable(char *RxBuf, uint8_t RxBufMaxLen)
{
  uint8_t Ret = 0;

  switch(somethingAvailable())
  {
    case RX_SERIAL_IDLE_NIBBLE_AVAILABLE:
    if(_MsgLen)
    {
      Ret = _MsgLen; /* End of Message */
      _MsgLen = 0;   /* Re-init */
    }
    break;
    
    case RX_SERIAL_CHAR_AVAILABLE:
    if(_MsgLen < RxBufMaxLen)
    {
      RxBuf[_MsgLen++] = _Char;
    }
    else
    {
      Ret = 1; /* Max Message length received */
    }
    break;
  }
  return(Ret);
}

uint8_t RcRxSerial::read()
{
   _available = 0;
   return(_Char);
}

uint8_t RcRxSerial::nibbleAvailable() /* Only for calibration purpose */
{
  return(_NibbleAvailable);
}

uint16_t RcRxSerial::lastWidth_us() /* Only for calibration purpose */
{
  _NibbleAvailable = 0;
  return(_LastWidth_us);
}

//========================================================================================================================
// PRIVATE FUNCTIONS
//========================================================================================================================
static int8_t PulseWithToNibbleIdx(uint16_t PulseWidth)
{
int8_t Ret = -1, Idx;

  for(Idx = 0; Idx < TABLE_ITEM_NBR(NibblePulse); Idx++)
  {
    if( (PulseWidth >= pgm_read_word(&NibblePulse[Idx].Min)) && (PulseWidth <= pgm_read_word(&NibblePulse[Idx].Max)) )
    {
      Ret = Idx;
      break;
    }
  }
  return(Ret);
}

