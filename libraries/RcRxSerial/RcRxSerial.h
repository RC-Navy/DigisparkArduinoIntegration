#ifndef RcRxSerial_h
#define RcRxSerial_h

/*
 English: by RC Navy (2012)
 =======
 <RcRxSerial>: a library to build an unidirectionnal serial port through RC Transmitter/Receiver.
 http://p.loussouarn.free.fr

 Francais: par RC Navy (2012)
 ========
 <RcRxSerial>: une bibliotheque pour construire un port serie unidirectionnel a travers un Emetteur/Recepteur RC.
 http://p.loussouarn.free.fr
*/

#include "Arduino.h"
#include <Rcul.h>

enum {RC_RX_SERIAL_SYNCH = 0, RC_RX_SERIAL_ASYNCH_RX1 = 0, RC_RX_SERIAL_ASYNCH_RX2, RC_RX_SERIAL_ASYNCH_RX3};

#define RC_RX_SERIAL_PENDING_NIBBLE_INDICATOR  (1 << 7)

typedef struct {
    uint16_t
            Asynch:     2, /* If Synch (Asynch = 0) 1 occurence of nibble validates the nibble, otherwise (1 + Asynch) occurences are expected */
            MsnPending: 1,
            Phase:      1,
            Available:  1,
            PrevValid:  5,
            Itself:     5;
    uint8_t 
            PrevIdx:    5, /* Prev Nibble to compare to the following one */
            SameCnt:    3; /* Current valid Nibble */
}RxNibbleSt_t;

class RcRxSerial
{
  private:
    Rcul         *_Rcul;
    uint8_t      _Ch;
    char         _Char;
    uint8_t      _MsgLen;
    uint16_t     _LastWidth_us;
    uint8_t      _available;
    RxNibbleSt_t _Nibble;
    uint8_t      somethingAvailable(void);
  public:
    RcRxSerial(Rcul *Rcul, uint8_t Asynch,   uint8_t Ch = 255);
    void         reassignRculSrc(Rcul *Rcul, uint8_t Ch = 255); /* Marginal use (do not use it, if you do not know what is it for) */
    uint8_t      available();
    uint8_t      read();
    uint8_t      msgAvailable(char *RxBuf,   uint8_t RxBufMaxLen);
    uint16_t     lastWidth_us();     /* Only for calibration purpose */
    uint8_t      nibbleAvailable();  /* Only for calibration purpose */
};

#endif

