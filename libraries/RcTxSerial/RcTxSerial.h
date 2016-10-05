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

/* MODULE CONFIGURATION */
//#define PPM_TX_SERIAL_USES_POWER_OF_2_AUTO_MALLOC /* Comment this if you set fifo size to a power of 2: this allows saving some bytes of program memory */

enum {RC_TX_SERIAL_INIT_WITH_DEFAULT=0, RC_TX_SERIAL_INIT_WITH_CURRENT_EEPROM};

enum {	RC_TX_SERIAL_MODE_NIBBLE_0=0, RC_TX_SERIAL_MODE_NIBBLE_4,
	RC_TX_SERIAL_MODE_NIBBLE_8,   RC_TX_SERIAL_MODE_NIBBLE_C, RC_TX_SERIAL_MODE_NIBBLE_I,
	RC_TX_SERIAL_MODE_NORMAL,     RC_TX_SERIAL_MODE_NB};

class RcTxSerial : public Stream
{
  private:
    // static data
    uint8_t  _Ch;
    uint8_t  _TxFifoSize;
    boolean  _TxCharInProgress;
    char    *_TxFifo;
    char     _TxChar;
    uint8_t  _TxFifoTail;
    uint8_t  _TxFifoHead;
    class    RcTxSerial *next;
    static   RcTxSerial *first;
    uint8_t  TxFifoRead(char *TxChar);
  public:
    RcTxSerial(Rcul *Rcul, uint8_t TxFifoSize, uint8_t Ch = 255);
    int peek();
    virtual size_t write(uint8_t byte);
    virtual int read();
    virtual int available();
    virtual void flush();
    using Print::write;
    static uint8_t process(); /* Send half character synchronized with every PPM frame */
};

#endif
