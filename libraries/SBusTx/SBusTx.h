/*
 English: by RC Navy (2016-2020)
 =======
 <SBusTx>: an asynchronous SBUS library. SBusTx is an SBUS frame generator.
 http://p.loussouarn.free.fr
 V1.0: initial release
 Francais: par RC Navy (2016-2020)
 ========
 <SBusTx>: une librairie SBUS asynchrone. SBusTx est un generateur de trame SBUS.
 http://p.loussouarn.free.fr
 V1.0: release initiale
*/
#ifndef SBUSTX_H
#define SBUSTX_H

#include "Arduino.h"
#include <Rcul.h>

#define SBUS_TX_VERSION                    1
#define SBUS_TX_REVISION                   0

#define SBUS_TX_NORMAL_TRAME_RATE_MS       14 /* Normal: 14ms */
#define SBUS_TX_HIGH_SPEED_TRAME_RATE_MS   7  /* High Speed: 7ms */

#define SBUS_TX_CH_NB                      16
#define SBUS_TX_DATA_NB                    ((((SBUS_TX_CH_NB * 11) + 7) / 8) + 1) /* +1 for flags -> 23 for 16 channels */

#define SBUS_TX_CH17                       (1 << 0)
#define SBUS_TX_CH18                       (1 << 1)
#define SBUS_TX_FRAME_LOST                 (1 << 2)
#define SBUS_TX_FAILSAFE                   (1 << 3)

class SBusTxClass : public Rcul
{
  private:
    Stream          *TxSerial;
    uint8_t          FrameRateMs;
    uint8_t          StartMs;
    uint8_t          Data[SBUS_TX_DATA_NB];
    uint8_t          Synchro;
  public:
    SBusTxClass(void);
    void             serialAttach(Stream *TxStream, uint8_t FrameRateMs = SBUS_TX_NORMAL_TRAME_RATE_MS);
    uint8_t          isSynchro(uint8_t SynchroClientIdx = 7); /* Default value: 8th Synchro client -> 0 to 6 free for other clients*/
    void             rawData(uint8_t Ch, uint16_t RawData);
    void             width_us(uint8_t Ch, uint16_t Width_us);
    void             flags(uint8_t FlagId, uint8_t FlagVal);
    void             sendChannels(void);
    /* Rcul support */
    virtual uint8_t  RculIsSynchro(uint8_t ClientIdx = RCUL_DEFAULT_CLIENT_IDX);
    virtual uint16_t RculGetWidth_us(uint8_t Ch);
    virtual void     RculSetWidth_us(uint16_t Width_us, uint8_t Ch = 255);
};

extern SBusTxClass SBusTx; /* Object externalisation */

#endif
