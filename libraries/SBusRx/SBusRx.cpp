#include <SBusRx.h>

enum {SBUS_WAIT_FOR_0x0F = 0, SBUS_WAIT_FOR_END_OF_DATA, SBUS_WAIT_FOR_0x00};

SBusRxClass SBusRx = SBusRxClass();

#define millis8()          (uint8_t)(millis() & 0x000000FF)
#define MAX_FRAME_TIME_MS  10

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
SBusRxClass::SBusRxClass()
{
  RxSerial = NULL;
  RxState  = SBUS_WAIT_FOR_0x0F;
  RxIdx    = 0;
  Synchro  = 0x00;
  StartMs  = millis8();
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
          RxIdx = 0;
          RxState = SBUS_WAIT_FOR_END_OF_DATA;
        }
        break;

        case SBUS_WAIT_FOR_END_OF_DATA:
        Data[RxIdx] = RxChar;
        RxIdx++;
        if(RxIdx >= SBUS_RX_DATA_NB) // 23
        {
          /* Check next byte is 0x00 */
          RxState = SBUS_WAIT_FOR_0x00;	
        }
        break;

        case SBUS_WAIT_FOR_0x00:
        if(RxChar == 0x00)
        {
          if(RxIdx == SBUS_RX_DATA_NB) // 23
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
  uint8_t  DataIdx = 0, DataBitIdx = 7, ChIdx = 0, ChBitIdx = 10;
  
  for(uint8_t GlobBitIdx = 0; GlobBitIdx < (SBUS_RX_CH_NB * 11); GlobBitIdx++)
  {
    bitWrite(Channel[ChIdx], ChBitIdx, bitRead(Data[DataIdx], DataBitIdx));
    DataBitIdx--; ChBitIdx--;
    if(DataBitIdx == 255)
    {
        DataBitIdx = 7;
        DataIdx++;
    }
    if(ChBitIdx == 255)
    {
        ChBitIdx = 10;
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

uint8_t SBusRxClass::isSynchro(uint8_t SynchroClientMsk /*= SBUS_CLIENT(7)*/)
{
  uint8_t Ret;
  
  Ret = !!(Synchro & SynchroClientMsk);
  if(Ret) Synchro &= ~SynchroClientMsk; /* Clear indicator for the Synchro client */
  
  return(Ret);
}

uint8_t SBusRxClass::flags(uint8_t FlagId)
{
  SBusFlagsSt_t *Flags;
  uint8_t Ret = 0;
  
  Flags = (SBusFlagsSt_t *)&Data[SBUS_RX_DATA_NB - 1];
  
  switch(FlagId)
  {
    case SBUS_RX_CH17:
    Ret = Flags->Ch17;
    break;
    
    case SBUS_RX_CH18:
    Ret = Flags->Ch18;
    break;
    
    case SBUS_RX_FRAME_LOST:
    Ret = Flags->FrameLost;
    break;
    
    case SBUS_RX_FAILSAFE:
    Ret = Flags->FailSafe;
    break;
  }
  return(Ret);
}

/* Rcul support */
uint8_t SBusRxClass::RculIsSynchro()
{
  return(isSynchro(SBUS_RX_CLIENT(6)));  
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
