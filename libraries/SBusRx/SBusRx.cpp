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
#include <SBusRx.h>

enum {SBUS_WAIT_FOR_0x0F = 0, SBUS_WAIT_FOR_END_OF_DATA, SBUS_WAIT_FOR_0x00};

SBusRxClass SBusRx = SBusRxClass();

#define millis8()           (uint8_t)(millis() & 0x000000FF)
#define MAX_FRAME_TIME_MS   10

#define SBUS_BIT_PER_CH     11

/*************************************************************************
                              GLOBAL VARIABLES
*************************************************************************/
/* Constructor */
SBusRxClass::SBusRxClass()
{
  RxSerial = NULL;
  RxState  = SBUS_WAIT_FOR_0x0F;
  RxIdx    = 0;
  Synchro  = 0x00;
  StartMs  = millis8();
  memset(Channel, 0, sizeof(Channel));
}

void SBusRxClass::serialAttach(Stream *RxStream)
{
  RxSerial = RxStream;
}

void SBusRxClass::process(void)
{
  uint8_t RxChar, Finished = 0;
  
  if(RxSerial)
  {
    if(millis8() - StartMs > MAX_FRAME_TIME_MS)
    {
      RxState = SBUS_WAIT_FOR_0x0F;
    }
    while(RxSerial->available() > 0)
    {
      RxChar = RxSerial->read();
      switch(RxState)
      {
        case SBUS_WAIT_FOR_0x0F:
        if(RxChar == 0x0F)
        {
          StartMs = millis8(); /* Start of frame */
          RxIdx = SBUS_RX_DATA_NB - 1; // Trick: store from the end
          RxState = SBUS_WAIT_FOR_END_OF_DATA;
        }
        break;

        case SBUS_WAIT_FOR_END_OF_DATA:
        Data[RxIdx] = RxChar;
        RxIdx--;
        if(RxIdx == 255)
        {
          /* Check next byte is 0x00 */
          RxState = SBUS_WAIT_FOR_0x00;
        }
        break;

        case SBUS_WAIT_FOR_0x00:
        if(RxChar == 0x00)
        {
          if(RxIdx == 255)
          {
            /* Data received with good synchro */
            updateChannels();
            Synchro  = 0xFF;
            Finished = 1;
          }
        }
        RxState = SBUS_WAIT_FOR_0x0F;
        break;
      }
      if(Finished) break;
    }
  }
}

void SBusRxClass::updateChannels(void)
{
  uint8_t  DataIdx = SBUS_RX_DATA_NB - 1, DataBitIdx = 0, ChIdx = 0, ChBitIdx = 0;

  for(uint8_t GlobBitIdx = 0; GlobBitIdx < (SBUS_RX_CH_NB * SBUS_BIT_PER_CH); GlobBitIdx++)
  {
    bitWrite(Channel[ChIdx], ChBitIdx, bitRead(Data[DataIdx], DataBitIdx));
    DataBitIdx++; ChBitIdx++;
    if(DataBitIdx == 8)
    {
        DataBitIdx = 0;
        DataIdx--;
    }
    if(ChBitIdx == SBUS_BIT_PER_CH)
    {
        ChBitIdx = 0;
        ChIdx++;
    }
  }

}

uint16_t SBusRxClass::rawData(uint8_t Ch)
{
  uint16_t RawData = 1024;
  
  if((Ch >= 1) && (Ch <= SBUS_RX_CH_NB))
  {
    Ch--;
    RawData = Channel[Ch];
  }
  
  return(RawData);
}

uint16_t SBusRxClass::width_us(uint8_t Ch)
{
  uint16_t Width_us = 1500;
  
  if((Ch >= 1) && (Ch <= SBUS_RX_CH_NB))
  {
    Ch--;
    Width_us = map(Channel[Ch], 0, 2047, 880, 2160);
  }
  
  return(Width_us);
}

uint8_t SBusRxClass::isSynchro(uint8_t SynchroClientIdx /*= 7*/)
{
  uint8_t Ret;
  
  Ret = !!(Synchro & RCUL_CLIENT_MASK(SynchroClientIdx));
  if(Ret) Synchro &= ~RCUL_CLIENT_MASK(SynchroClientIdx); /* Clear indicator for the Synchro client */
  
  return(Ret);
}

uint8_t SBusRxClass::flags(uint8_t FlagId)
{
  return(!!(Data[0] & FlagId));
}

/* Rcul support */
uint8_t SBusRxClass::RculIsSynchro(uint8_t ClientIdx /*= RCUL_DEFAULT_CLIENT_IDX*/)
{
  return(isSynchro(ClientIdx));  
}

uint16_t SBusRxClass::RculGetWidth_us(uint8_t Ch)
{
  return(width_us(Ch));
}

void SBusRxClass::RculSetWidth_us(uint16_t Width_us, uint8_t Ch /*= 255*/)
{
  Width_us = Width_us; /* To avoid a compilation warning */
  Ch = Ch;             /* To avoid a compilation warning */
}
