#ifndef SRXLRX_H
#define SRXLRX_H

#include "Arduino.h"
#include <Rcul.h>

#define SRXL_RX_A1_CH_NB    12
#define SRXL_RX_A2_CH_NB    16

#define SRXL_MAX_RX_CH_NB   SRXL_RX_A2_CH_NB

class SrxlRxClass : public Rcul
{
  private:
    Stream               *RxSerial;
    uint8_t               StartMs;
    uint8_t               RxState;
    int8_t                RxIdx;
    uint8_t               ChNb;
    uint16_t              RawData[SRXL_MAX_RX_CH_NB];
    uint16_t              Channel[SRXL_MAX_RX_CH_NB];
    uint16_t              ComputedCrc;
    uint8_t               Synchro;
  public:
    SrxlRxClass(void);
    void                  serialAttach(Stream *RxStream);
    void                  process(void);
    uint8_t               isSynchro(uint8_t SynchroClientIdx = 7); /* Default value: 8th Synchro client -> 0 to 6 free for other clients*/
    uint16_t              rawData(uint8_t Ch);
    uint16_t              width_us(uint8_t Ch);
    uint8_t               channelNb(void);
    /* Rcul support */
    virtual uint8_t       RculIsSynchro(uint8_t ClientIdx = RCUL_DEFAULT_CLIENT_IDX);
    virtual uint16_t      RculGetWidth_us(uint8_t Ch);
    virtual void          RculSetWidth_us(uint16_t Width_us, uint8_t Ch = 255);
};

extern SrxlRxClass SrxlRx; /* Object externalisation */

#endif
