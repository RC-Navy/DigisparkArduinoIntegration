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
#include <SBusTx.h>

SBusTxClass SBusTx = SBusTxClass();

#define millis8()                 (uint8_t)(millis() & 0x000000FF)

#define SBUS_TX_START_BYTE        0x0F
#define SBUS_TX_END_BYTE          0x00

#define SBUS_BIT_PER_CH           11

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

uint8_t SBusTxClass::isSynchro(uint8_t SynchroClientIdx /*= 7*/)
{
  uint8_t Ret;
  
  if(!Synchro)
  {
    if(millis8() - StartMs >= FrameRateMs)
    {
      Synchro = 0xFF;
    }
  }
  Ret = !!(Synchro & RCUL_CLIENT_MASK(SynchroClientIdx));
  if(Ret) Synchro &= ~RCUL_CLIENT_MASK(SynchroClientIdx); /* Clear indicator for the Synchro client */
  
  return(Ret);
}

void SBusTxClass::rawData(uint8_t Ch, uint16_t RawData)
{
  uint8_t  ChIdx, RevGlobBitIdx, FirstByteIdx, StartFirstBit, ByteNb = 2, LocBitIdx;
  uint8_t  ChByte[3]; /* Working buffer */

  if((Ch >= 1) && (Ch <= SBUS_TX_CH_NB))
  {
    /* Here, we have to imagine Data[] is in reversed order */
    /* Let's search the 2 or 3 bytes which contain the SBUS_BIT_PER_CH (11) bits of the Channel */
    ChIdx         = Ch - 1;
    RevGlobBitIdx = (ChIdx * SBUS_BIT_PER_CH);
    FirstByteIdx  = RevGlobBitIdx / 8;
    StartFirstBit = RevGlobBitIdx % 8;
    ByteNb       += (StartFirstBit > (2 * 8) - SBUS_BIT_PER_CH);
    /* Load the 2 or 3 Data bytes in ChByte[] */
    for(uint8_t Idx = 0; Idx < ByteNb; Idx++)
    {
      ChByte[Idx] = Data[FirstByteIdx + (ByteNb - 1) - Idx];
    }
    /* Load RawData in ChByte[] */
    LocBitIdx = StartFirstBit;
    for(uint8_t Idx = 0; Idx < SBUS_BIT_PER_CH; Idx++)
    {
      /* Write bits from left to right */
      bitWrite(ChByte[ByteNb - 1 - (LocBitIdx / 8)], (LocBitIdx % 8), bitRead(RawData, Idx));
      LocBitIdx++;
    }
    /* Load the 2 or 3 ChByte[] bytes in Data[] */
    for(uint8_t Idx = 0; Idx < ByteNb; Idx++)
    {
      Data[FirstByteIdx + (ByteNb - 1) - Idx] = ChByte[Idx];
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
  if(FlagVal) Data[SBUS_TX_DATA_NB - 1] |=  FlagId;
  else        Data[SBUS_TX_DATA_NB - 1] &= ~FlagId;
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
uint8_t SBusTxClass::RculIsSynchro(uint8_t ClientIdx /*= RCUL_DEFAULT_CLIENT_IDX*/)
{
  return(isSynchro(ClientIdx));
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
