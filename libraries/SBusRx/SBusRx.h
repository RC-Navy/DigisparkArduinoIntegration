/*
 English: by RC Navy (2016-2020)
 =======
 <SBusRx>: an asynchronous SBUS library. SBusRx is an SBUS frame decoder.
 http://p.loussouarn.free.fr
 V1.0: initial release
 Francais: par RC Navy (2016-2020)
 ========
 <SBusRx>: une librairie SBUS asynchrone. SBusRx est un decodeur de trame SBUS.
 http://p.loussouarn.free.fr
 V1.0: release initiale
*/
#ifndef SBUSRX_H
#define SBUSRX_H

#include "Arduino.h"
#include <Rcul.h>

#define SBUS_RX_VERSION     1
#define SBUS_RX_REVISION    0

#define SBUS_RX_CH_NB       16
#define SBUS_RX_DATA_NB     ((((SBUS_RX_CH_NB * 11) + 7) / 8) + 1) /* +1 for flags -> 23 for 16 channels */

#define SBUS_RX_CH17        (1 << 0)
#define SBUS_RX_CH18        (1 << 1)
#define SBUS_RX_FRAME_LOST  (1 << 2)
#define SBUS_RX_FAILSAFE    (1 << 3)

class SBusRxClass : public Rcul
{
  private:
    Stream            *RxSerial;
    uint8_t           StartMs;
    uint8_t           RxState;
    uint8_t           RxIdx;
    int8_t            Data[SBUS_RX_DATA_NB]; /* +1 for flags */
    uint16_t          Channel[SBUS_RX_CH_NB];
    uint8_t           Synchro;
    void              updateChannels(void);
  public:
    SBusRxClass(void);
    void              serialAttach(Stream *RxStream);
    void              process(void);
    uint8_t           isSynchro(uint8_t SynchroClientIdx = 7); /* Default value: 8th Synchro client -> 0 to 6 free for other clients*/
    uint16_t          rawData(uint8_t Ch);
    uint16_t          width_us(uint8_t Ch);
    uint8_t           flags(uint8_t FlagId);
    /* Rcul support */
    virtual uint8_t   RculIsSynchro(uint8_t ClientIdx = RCUL_DEFAULT_CLIENT_IDX);
    virtual uint16_t  RculGetWidth_us(uint8_t Ch);
    virtual void      RculSetWidth_us(uint16_t Width_us, uint8_t Ch = 255);
};

extern SBusRxClass SBusRx; /* Object externalisation */

#endif
