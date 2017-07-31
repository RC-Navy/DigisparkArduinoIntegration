#include <RcTxSerial.h>

/*
 English: by RC Navy (2015)
 =======
 <RcTxSerial>: a library to build an unidirectionnal serial port through RC Transmitter/Receiver.
 http://p.loussouarn.free.fr

 Francais: par RC Navy (2015)
 ========
 <RcTxSerial>: une librairie pour construire un port serie a travers un Emetteur/Recepteur RC.
 http://p.loussouarn.free.fr
*/

enum {NIBBLE_0=0, NIBBLE_1, NIBBLE_2, NIBBLE_3, NIBBLE_4, NIBBLE_5, NIBBLE_6, NIBBLE_7, NIBBLE_8, NIBBLE_9, NIBBLE_A, NIBBLE_B, NIBBLE_C, NIBBLE_D, NIBBLE_E, NIBBLE_F, NIBBLE_I, NIBBLE_NB};
/*
        1024us                                                          2048us
          |---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
NIBBLE    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   I
*/

#define PULSE_WIDTH_US_MIN            1024
#define PULSE_WIDTH_US(NibbleIdx)     (PULSE_WIDTH_US_MIN + ((NibbleIdx) * 64)) /* 2048 */

#define GET_PULSE_WIDTH_US(NibbleIdx) (uint16_t)pgm_read_word(&PulseWidth[(NibbleIdx)])

const uint16_t PulseWidth[] PROGMEM = {PULSE_WIDTH_US(NIBBLE_0), PULSE_WIDTH_US(NIBBLE_1), PULSE_WIDTH_US(NIBBLE_2), PULSE_WIDTH_US(NIBBLE_3),
                                       PULSE_WIDTH_US(NIBBLE_4), PULSE_WIDTH_US(NIBBLE_5), PULSE_WIDTH_US(NIBBLE_6), PULSE_WIDTH_US(NIBBLE_7),
                                       PULSE_WIDTH_US(NIBBLE_8), PULSE_WIDTH_US(NIBBLE_9), PULSE_WIDTH_US(NIBBLE_A), PULSE_WIDTH_US(NIBBLE_B),
                                       PULSE_WIDTH_US(NIBBLE_C), PULSE_WIDTH_US(NIBBLE_D), PULSE_WIDTH_US(NIBBLE_E), PULSE_WIDTH_US(NIBBLE_F),
                                       PULSE_WIDTH_US(NIBBLE_I)};

static Rcul   *_Rcul = NULL; /* All the PpmTxSerial have the same TinyPpmGen */
RcTxSerial    *RcTxSerial::first = NULL;

/*************************************************************************
                           GLOBAL VARIABLES
*************************************************************************/

/* Constructor */
RcTxSerial::RcTxSerial(Rcul *Rcul, uint8_t TxFifoSize, uint8_t Ch /* = 255 */)
{
#ifdef PPM_TX_SERIAL_USES_POWER_OF_2_AUTO_MALLOC
  if(TxFifoSize > 128) TxFifoSize = 128; /* Must fit in a 8 bits  */
  _TxFifoSize = 1;
  do
  {
    _TxFifoSize <<= 1;
  }while(_TxFifoSize < TxFifoSize); /* Search for the _TxFifoSize in power of 2 just greater or equal to requested size */
#else
  _TxFifoSize = TxFifoSize;
#endif
  _TxFifo = (char *)malloc(_TxFifoSize);
  if(_TxFifo != NULL)
  {
    _Rcul = Rcul;
    _Ch     = Ch;
    _TxCharInProgress = 0;
    _TxFifoTail = 0;
    _TxFifoHead = 0;
    next  = first;
    first = this;
  }
}

size_t RcTxSerial::write(uint8_t b)
{
  size_t Ret = 0;

  // if buffer full, discard the character and return
  if ((_TxFifoTail + 1) % _TxFifoSize != _TxFifoHead)
  {
    // save new data in buffer: tail points to where byte goes
    _TxFifo[_TxFifoTail] = b; // save new byte
    _TxFifoTail = (_TxFifoTail + 1) % _TxFifoSize;
    Ret = 1;
  }
  return(Ret);
}

int RcTxSerial::read()
{
  return -1;
}

int RcTxSerial::available()
{
  return 0;
}

void RcTxSerial::flush()
{
  _TxFifoHead = _TxFifoTail = 0;
}

int RcTxSerial::peek()
{
  // Empty buffer?
  if (_TxFifoHead == _TxFifoTail)
    return -1;

  // Read from "head"
  return _TxFifo[_TxFifoHead];
}

uint8_t RcTxSerial::process()
{
  uint8_t  Ret = 0;
  uint16_t PulseWidth_us;
  RcTxSerial *t;

  if(_Rcul->RculIsSynchro())
  {
    for ( t = first; t != 0; t = t->next )
    {
      if(t->_TxCharInProgress)
      {
	PulseWidth_us = GET_PULSE_WIDTH_US(t->_TxChar & 0x0F); /* LSN */
	t->_TxCharInProgress = 0;
      }
      else
      {
	if(t->TxFifoRead(&t->_TxChar))
	{
	  PulseWidth_us = GET_PULSE_WIDTH_US((t->_TxChar & 0xF0) >> 4); /* MSN first */
	  t->_TxCharInProgress = 1;
	}
	else
	{
	  PulseWidth_us = GET_PULSE_WIDTH_US(NIBBLE_I); /* Noting to transmit */
	}
      }
      _Rcul->RculSetWidth_us(PulseWidth_us, t->_Ch); /* /!\ Ch as last argument /!\ */
    }
    Ret = 1;
  }
  return(Ret);
}

//========================================================================================================================
// PRIVATE FUNCTIONS
//========================================================================================================================
uint8_t RcTxSerial::TxFifoRead(char *TxChar)
{
uint8_t Ret = 0;
  // Empty buffer?
  if (_TxFifoHead != _TxFifoTail)
  {
    // Read from "head"
    *TxChar = _TxFifo[_TxFifoHead]; // grab next byte
    _TxFifoHead = (_TxFifoHead + 1) % _TxFifoSize;
    Ret=1;
  }
  return(Ret);
}
