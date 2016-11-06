#ifndef SBUSRX_H
#define SBUSRX_H

#include "Arduino.h"
#include <Rcul.h>

#define SBUS_RX_CH_NB    16
#define SBUS_RX_DATA_NB  ((((SBUS_RX_CH_NB * 11) + 7) / 8) + 1) /* +1 for flags -> 23 for 16 channels */

enum {SBUS_RX_CH17 = 0, SBUS_RX_CH18, SBUS_RX_FRAME_LOST, SBUS_RX_FAILSAFE, SBUS_RX_FLAG_NB};

/* Macro for Reader client */
#define SBUS_RX_CLIENT(ClientIdx)     (1 << (ClientIdx)) /* Range: 0 to 7 */

class SBusRxClass : public Rcul
{
  private:
    Stream               *RxSerial;
    uint8_t               StartMs;
    uint8_t               RxState;
    uint8_t               RxIdx;
    int8_t                Data[SBUS_RX_DATA_NB]; /* +1 for flags */
    uint16_t              Channel[SBUS_RX_CH_NB];
    uint8_t               Synchro;
    void                  updateChannels(void);
  public:
    SBusRxClass(void);
    void     serialAttach(Stream *RxStream);
    void     process(void);
    uint8_t  isSynchro(uint8_t SynchroClientMsk = SBUS_RX_CLIENT(7)); /* Default value: 8th Synchro client -> 0 to 6 free for other clients*/
    uint16_t rawData(uint8_t Ch);
    uint16_t width_us(uint8_t Ch);
    uint8_t  flags(uint8_t FlagId);
    /* Rcul support */
    virtual uint8_t  RculIsSynchro();
    virtual uint16_t RculGetWidth_us(uint8_t Ch);
    virtual void     RculSetWidth_us(uint16_t Width_us, uint8_t Ch = 255);
};

extern SBusRxClass SBusRx; /* Object externalisation */

#endif
