#ifndef RcTxSerial_h
#define RcTxSerial_h

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

#include "Arduino.h"
#include <Rcul.h>

#include <inttypes.h>
#include <Stream.h>

#define RC_TX_TIME_MS(PpmPeriodUs, SynchAsynch, ByteNb)  ( ( ((2 * (ByteNb)) * (SynchAsynch)) + 2) * ( ((PpmPeriodUs) + 500) / 1000 ) ) /* Majored */

/* MODULE CONFIGURATION */
#define PPM_TX_SERIAL_USES_POWER_OF_2_AUTO_MALLOC /* Comment this if you set fifo size to a power of 2: this allows saving some bytes of program memory */

enum {RC_TX_SERIAL_SYNCH = 1, RC_TX_SERIAL_ASYNCH3 = 3, RC_TX_SERIAL_ASYNCH4 = 4, RC_TX_SERIAL_ASYNCH5 = 5};

typedef struct {
    uint8_t
            TxInProgress:     1, /*  */
            TxCharInProgress: 1;
    uint16_t
            NbToSend:         3,
            SentCnt:          3,
            CurIdx:           5, /* Prev Nibble to compare to the following one */
            PrevIdx:          5; /* Prev Nibble to compare to the following one */
}TxNibbleSt_t;

class RcTxSerial : public Stream
{
  private:
    // static data
    uint8_t      _Ch;
    uint8_t      _TxFifoSize;
    char        *_TxFifo;
    char         _TxChar;
    uint8_t      _TxFifoTail;
    uint8_t      _TxFifoHead;
    TxNibbleSt_t _Nibble;
    class        RcTxSerial *next;
    static       RcTxSerial *first;
    uint8_t      TxFifoRead(char *TxChar);
  public:
    RcTxSerial(Rcul *Rcul, uint8_t NbToSend, uint8_t TxFifoSize, uint8_t Ch = 255);
    int peek();
    virtual size_t write(uint8_t byte);
    virtual int read();
    virtual int available();
    virtual void flush();
    using Print::write;
    static uint8_t process(); /* Send half character synchronized with every PPM frame */
};

#endif
