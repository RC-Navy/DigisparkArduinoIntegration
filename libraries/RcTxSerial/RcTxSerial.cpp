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

/*
NIBBLE_WIDTH_US
  <--->
 996                                                                     2004
  |-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|-+-|
    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F   R   I
    <--->
    |   |                                                               |
  1024 1080                                                           1976
INTER_NIBBLE
*/
enum {NIBBLE_0=0, NIBBLE_1, NIBBLE_2, NIBBLE_3, NIBBLE_4, NIBBLE_5, NIBBLE_6, NIBBLE_7, NIBBLE_8, NIBBLE_9, NIBBLE_A, NIBBLE_B, NIBBLE_C, NIBBLE_D, NIBBLE_E, NIBBLE_F, NIBBLE_R, NIBBLE_I, NIBBLE_NB};

#define NEUTRAL_WIDTH_US            1500
#define NIBBLE_WIDTH_US             56
#define FULL_EXCURSION_US           (NIBBLE_WIDTH_US * NIBBLE_NB)
#define PULSE_MIN_US                (NEUTRAL_WIDTH_US - (FULL_EXCURSION_US / 2))
#define PULSE_WIDTH_US(NibbleIdx)   (PULSE_MIN_US + (NIBBLE_WIDTH_US / 2)+ ((NibbleIdx) * NIBBLE_WIDTH_US))

#define GET_PULSE_WIDTH_US(NibbleIdx) (uint16_t)pgm_read_word(&PulseWidth[(NibbleIdx)])

const uint16_t PulseWidth[] PROGMEM = {PULSE_WIDTH_US(NIBBLE_0), PULSE_WIDTH_US(NIBBLE_1), PULSE_WIDTH_US(NIBBLE_2), PULSE_WIDTH_US(NIBBLE_3),
                                       PULSE_WIDTH_US(NIBBLE_4), PULSE_WIDTH_US(NIBBLE_5), PULSE_WIDTH_US(NIBBLE_6), PULSE_WIDTH_US(NIBBLE_7),
                                       PULSE_WIDTH_US(NIBBLE_8), PULSE_WIDTH_US(NIBBLE_9), PULSE_WIDTH_US(NIBBLE_A), PULSE_WIDTH_US(NIBBLE_B),
                                       PULSE_WIDTH_US(NIBBLE_C), PULSE_WIDTH_US(NIBBLE_D), PULSE_WIDTH_US(NIBBLE_E), PULSE_WIDTH_US(NIBBLE_F),
                                       PULSE_WIDTH_US(NIBBLE_R), PULSE_WIDTH_US(NIBBLE_I)};

static Rcul   *_Rcul = NULL;
RcTxSerial    *RcTxSerial::first = NULL;

/*************************************************************************
                           GLOBAL VARIABLES
*************************************************************************/

/* Constructor */
RcTxSerial::RcTxSerial(Rcul *Rcul, uint8_t Asynch, uint8_t TxFifoSize, uint8_t Ch /* = 255 */)
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
    _Rcul                    = Rcul;
    _Nibble.NbToSend         = Asynch + 1;
    _Nibble.SentCnt          = 0;
    _Nibble.TxInProgress     = 0;
    _Nibble.TxCharInProgress = 0;
    _Ch                      = Ch;
    _TxFifoTail              = 0;
    _TxFifoHead              = 0;
    next                     = first;
    first                    = this;
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
  uint8_t    Ret = 0;
  RcTxSerial *t;
  if(_Rcul->RculIsSynchro())
  {
    for (t = first; t != 0; t = t->next)
    {
      if(!t->_Nibble.TxInProgress)
      {
        t->_Nibble.TxInProgress = 1;
        if(!t->_Nibble.TxCharInProgress)
        {
          /* Get next char to send */
          if(t->TxFifoRead(&t->_TxChar))
          {
            t->_Nibble.CurIdx = ((t->_TxChar & 0xF0) >> 4); /* MSN first */
            t->_Nibble.TxCharInProgress = 1;
          }
          else
          {
            t->_Nibble.CurIdx = NIBBLE_I; /* Nothing to transmit */
          }
        }
        else
        {
          /* Tx Char in progress: send least significant nibble */
          t->_Nibble.CurIdx = t->_TxChar & 0x0F; /* LSN */
          t->_Nibble.TxCharInProgress = 0;
        }
        if(t->_Nibble.CurIdx == t->_Nibble.PrevIdx) t->_Nibble.CurIdx = NIBBLE_R; /* Repeat symbol */
        t->_Nibble.PrevIdx = t->_Nibble.CurIdx;
      }
      /* Send the Nibble or the Repeat or the Idle symbol */
      _Rcul->RculSetWidth_us(GET_PULSE_WIDTH_US(t->_Nibble.CurIdx), t->_Ch); /* /!\ Ch as last argument /!\ */
      t->_Nibble.SentCnt++;
      if(t->_Nibble.SentCnt >= t->_Nibble.NbToSend)
      {
        t->_Nibble.SentCnt = 0;
        t->_Nibble.TxInProgress = 0;
      }
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
