#include <SBusTx.h>

SBusTxClass SBusTx = SBusTxClass();

#define millis8()                 (uint8_t)(millis() & 0x000000FF)

#define SBUS_TX_START_BYTE        0x0F
#define SBUS_TX_END_BYTE          0x00

typedef struct {
  uint8_t
          Reserved:   4, /* Bit 0 to 3 */
          FailSafe:   1, /* Bit 4 */
          FrameLost:  1, /* Bit 5 */
          Ch18:       1, /* Bit 6 */
          Ch17:       1; /* Bit 7 */
}SBusFlagsSt_t;

/*************************************************************************
                              GLOBAL VARIABLES
*************************************************************************/
/* Constructor */
SBusTxClass::SBusTxClass()
{
  TxSerial = NULL;
  Synchro  = 0x00;
}

void SBusTxClass::serialAttach(Stream *TxStream, uint8_t FrameRateMs/* = SBUS_TX_NORMAL_TRAME_RATE_MS*/)
{
  TxSerial = TxStream;
  this->FrameRateMs = FrameRateMs;
}

uint8_t SBusTxClass::isSynchro(uint8_t SynchroClientMsk /*= SBUS_CLIENT(7)*/)
{
  uint8_t Ret;
  
  if(!Synchro)
  {
    if(millis8() - StartMs >= FrameRateMs)
    {
      Synchro = 0xFF;
    }
  }
  Ret = !!(Synchro & SynchroClientMsk);
  if(Ret) Synchro &= ~SynchroClientMsk; /* Clear indicator for the Synchro client */
  
  return(Ret);
}

void SBusTxClass::rawData(uint8_t Ch, uint16_t RawData)
{
  uint8_t  ChIdx, GlobStartBitIdx, DataByteIdx, DataBitIdx, RawDataBitIdx;
  
  if((Ch >= 1) && (Ch <= SBUS_TX_CH_NB))
  {
    ChIdx           = Ch - 1;
    GlobStartBitIdx = ChIdx * 11;
    DataByteIdx     = GlobStartBitIdx / 8;
    DataBitIdx      = GlobStartBitIdx % 8;
    for(RawDataBitIdx = 0; RawDataBitIdx < 11; RawDataBitIdx++)
    {
      bitWrite(Data[DataByteIdx + ((DataBitIdx + RawDataBitIdx) / 8)], 7 - ((DataBitIdx + RawDataBitIdx) % 8), bitRead(RawData, 10 - RawDataBitIdx));
    }
  }

}

void SBusTxClass::width_us(uint8_t Ch, uint16_t Width_us)
{
  uint16_t RawData;
  
  if((Ch >= 1) && (Ch <= SBUS_TX_CH_NB))
  {
    Width_us = constrain(Width_us, 880, 2160);
    RawData = map(Width_us, 880, 2160, 0, 2047);
    RawData = constrain(RawData, 0, 2047);
    rawData(Ch, RawData);
  }
}

void SBusTxClass::flags(uint8_t FlagId, uint8_t FlagVal)
{
  SBusFlagsSt_t *Flags;
  
  Flags = (SBusFlagsSt_t *)&Data[SBUS_TX_DATA_NB - 1];
  
  switch(FlagId)
  {
    case SBUS_TX_CH17:
    Flags->Ch17 = !!FlagVal;
    break;
    
    case SBUS_TX_CH18:
    Flags->Ch18 = !!FlagVal;
    break;
    
    case SBUS_TX_FRAME_LOST:
    Flags->FrameLost = !!FlagVal;
    break;
    
    case SBUS_TX_FAILSAFE:
    Flags->FailSafe = !!FlagVal;
    break;
  }
  
}

void SBusTxClass::sendChannels(void)
{
  TxSerial->write((uint8_t)SBUS_TX_START_BYTE);
  TxSerial->write(Data, SBUS_TX_DATA_NB);
  TxSerial->write((uint8_t)SBUS_TX_END_BYTE);
  Synchro = 0;
  StartMs = millis8();
}

/* Rcul support */
uint8_t SBusTxClass::RculIsSynchro()
{
  return(isSynchro(SBUS_TX_CLIENT(6)));  
}

uint16_t SBusTxClass::RculGetWidth_us(uint8_t Ch)
{
  Ch = Ch;             /* To avoid a compilation warning */
  return(1500);        /* Fake */
}

void SBusTxClass::RculSetWidth_us(uint16_t Width_us, uint8_t Ch /*= 255*/)
{
  width_us(Ch, Width_us);
}
