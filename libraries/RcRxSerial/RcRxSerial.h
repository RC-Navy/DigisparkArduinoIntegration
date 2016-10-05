#ifndef RcRxSerial_h
#define RcRxSerial_h

/*
 English: by RC Navy (2012)
 =======
 <RcRxSerial>: a library to build an unidirectionnal serial port through RC Transmitter/Receiver.
 http://p.loussouarn.free.fr

 Francais: par RC Navy (2012)
 ========
 <RcRxSerial>: une librairie pour construire un port serie a travers un Emetteur/Recepteur RC.
 http://p.loussouarn.free.fr
*/

#include "Arduino.h"
#include <Rcul.h>

class RcRxSerial
{
  private:
    Rcul    *_Rcul;
    uint8_t  _Ch;
    char     _Char;
    uint8_t  _MsgLen;
    uint16_t _LastWidth_us;
    boolean  _available;
    boolean  _Nibble;
    boolean  _NibbleAvailable;
    uint8_t  somethingAvailable(void);
  public:
    RcRxSerial(Rcul *Rcul, uint8_t Ch = 255);
    uint8_t  available();
    uint8_t  msgAvailable(char *RxBuf, uint8_t RxBufMaxLen);
    uint8_t  read();
    void     reassignRculSrc(Rcul *Rcul, uint8_t Ch = 255); /* Marginal use (do not use it, if you do not know what is it for) */
    uint16_t lastWidth_us();     /* Only for calibration purpose */
    uint8_t  nibbleAvailable();  /* Only for calibration purpose */
};

#endif

