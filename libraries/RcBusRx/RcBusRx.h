#ifndef RCBUSRX_H
#define RCBUSRX_H

#include "Arduino.h"
#include <Rcul.h>

/* Library Version/Revision */
#define SBUS_RX_VERSION       0
#define SBUS_RX_REVISION      1

/* Compilation directives to reduce code size in case of small micro-controller (comment unused protocols) */
#define SBUS_RX_SBUS_SUPPORT  1 // <-- Set here 1 for support or 0 to not support it
#define SBUS_RX_SRXL_SUPPORT  1 // <-- Set here 1 for support or 0 to not support it
#define SBUS_RX_SUMD_SUPPORT  1 // <-- Set here 1 for support or 0 to not support it
#define SBUS_RX_IBUS_SUPPORT  1 // <-- Set here 1 for support or 0 to not support it
#define SBUS_RX_JETI_SUPPORT  1 // <-- Set here 1 for support or 0 to not support it

enum {RC_BUS_RX_SBUS = 0, RC_BUS_RX_SRXL, RC_BUS_RX_SUMD, RC_BUS_RX_IBUS, RC_BUS_RX_JETI, RC_BUS_RX_NB};

/* SBUS */
/* /!\ Serial port shall be set to 100000, SERIAL_8E2 /!\ */
#define SBUS_RX_SERIAL_CFG    100000, SERIAL_8E2
#define SBUS_RX_CH_NB         16
#define SBUS_RX_DATA_NB       ((((SBUS_RX_CH_NB * 11) + 7) / 8) + 1) /* +1 for flags -> 23 for 16 channels */

#define SBUS_RX_CH17          (1 << 0)
#define SBUS_RX_CH18          (1 << 1)
#define SBUS_RX_FRAME_LOST    (1 << 2)
#define SBUS_RX_FAILSAFE      (1 << 3)

/* SRXL */
/* /!\ Serial port shall be set to 115200, SERIAL_8E2 and needs often a serial inverter /!\ */
#define SRXL_RX_SERIAL_CFG    115200
#define SRXL_RX_A1_CH_NB      12
#define SRXL_RX_A2_CH_NB      16

#define SRXL_MAX_RX_CH_NB     SRXL_RX_A2_CH_NB

/* SUMD */
#define SUMD_RX_SERIAL_CFG    115200 // Frame send every 10 ms
#define SUMD_RX_FAILSAFE      (1 << 7)

/* IBUS */
#define IBUS_RX_SERIAL_CFG    115200 // Frame send every 7 ms
#define IBUS_RX_CH_NB         14

/* JETI */
#define JETI_RX_SERIAL_CFG    125000 // Frame send every ? ms

typedef struct{
  uint8_t
        Proto     :4,
        FailSafe  :1,
        Reserved  :3; // For SUMD FailSafe
}RcBusInfoSt_t;

class RcBusRxClass : public Rcul
{
  public:
    RcBusRxClass(void);
    void                  serialAttach(Stream *RxStream);
    void                  setProto(uint8_t Proto);
    void                  process(void);
    uint8_t               isSynchro(uint8_t SynchroClientIdx = 7); /* Default value: 8th Synchro client -> 0 to 6 free for other clients*/
    uint16_t              rawData(uint8_t Ch);
    uint16_t              width_us(uint8_t Ch);
    uint8_t               channelNb(void);
    uint8_t               flags(uint8_t FlagId);
    /* Rcul support */
    virtual uint8_t       RculIsSynchro(uint8_t ClientIdx = RCUL_DEFAULT_CLIENT_IDX);
    virtual uint16_t      RculGetWidth_us(uint8_t Ch);
    virtual void          RculSetWidth_us(uint16_t Width_us, uint8_t Ch = 255);
  private:
    Stream               *RxSerial;
    RcBusInfoSt_t         Info;
    uint8_t               StartMs;
    uint8_t               RxState;
    uint8_t               RxIdx;
    uint8_t               ChNb;
    uint8_t               RawData[SRXL_MAX_RX_CH_NB * 2]; // Max size
    uint16_t              Channel[SRXL_MAX_RX_CH_NB];     // Max size
    uint16_t              ComputedCrc; // CRC or Checksum
    uint8_t               Synchro;
    void                  updateChannels(void); // Only needed for SBUS
};

extern RcBusRxClass RcBusRx; /* Object externalisation */

#endif
