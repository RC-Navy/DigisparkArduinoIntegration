#include <SrxlRx.h>

enum {SRXL_WAIT_FOR_0xA1_OR_0xA2 = 0, SRXL_WAIT_FOR_CHANNELS, SRXL_WAIT_FOR_CHECKSUM};

SrxlRxClass SrxlRx = SrxlRxClass();

static uint16_t crc16_CCITT(uint16_t crc, uint8_t value);

#define millis8()          (uint8_t)(millis() & 0x000000FF)
#define MAX_FRAME_TIME_MS  10

/*************************************************************************
                              GLOBAL VARIABLES
*************************************************************************/
/* Constructor */
SrxlRxClass::SrxlRxClass()
{
  RxSerial = NULL;
  RxState  = SRXL_WAIT_FOR_0xA1_OR_0xA2;
  Synchro  = 0x00;
}

void SrxlRxClass::serialAttach(Stream *RxStream)
{
  RxSerial = RxStream;
}

void SrxlRxClass::process(void)
{
  uint8_t  RxChar, Finished = 0;
  uint8_t *BytePtr = (uint8_t *)RawData;
  
  if(RxSerial)
  {
    if(millis8() - StartMs > MAX_FRAME_TIME_MS)
    {
      StartMs = millis8();
      RxState = SRXL_WAIT_FOR_0xA1_OR_0xA2;
    }
    while(RxSerial->available() > 0)
    {
      StartMs = millis8();
      RxChar = RxSerial->read();
      switch(RxState)
      {
        case SRXL_WAIT_FOR_0xA1_OR_0xA2:
        if((RxChar == 0xA1) || (RxChar == 0xA2))
        {
          ChNb = (RxChar == 0xA1)? SRXL_RX_A1_CH_NB: SRXL_RX_A2_CH_NB;
          RxIdx = 0;
          ComputedCrc = 0;
          ComputedCrc = crc16_CCITT(ComputedCrc, RxChar);
          RxState = SRXL_WAIT_FOR_CHANNELS;
        }
        break;

        case SRXL_WAIT_FOR_CHANNELS:
        BytePtr[RxIdx++] = RxChar;
        ComputedCrc = crc16_CCITT(ComputedCrc, RxChar);
        if(RxIdx >= (2 * ChNb))
        {
          RxState = SRXL_WAIT_FOR_CHECKSUM;
        }
        break;

        case SRXL_WAIT_FOR_CHECKSUM:
        RxIdx++; /* Rx Checksum is not stored: just increment index */
        ComputedCrc = crc16_CCITT(ComputedCrc, RxChar);
        if(RxIdx >= ((2 * ChNb) + 2))
        {
          if(!ComputedCrc)
          {
            /* Update channels from raw data */
            for(uint8_t Idx = 0; Idx < ChNb; Idx++)
            {
              Channel[Idx] = map(RawData[Idx], 0, 0x0FFF, 800, 2200);
            }
            Synchro  = 0xFF;
            Finished = 1;
          }
          RxState = SRXL_WAIT_FOR_0xA1_OR_0xA2;
        }
        break;
      }
      if(Finished) break;
    }
  }
}

uint8_t SrxlRxClass::channelNb(void)
{
  return(ChNb);
}

uint16_t SrxlRxClass::rawData(uint8_t Ch)
{
  uint16_t OneRawData = 0x0800; /* Corresponds to 1500 us */
  
  if((Ch >= 1) && (Ch <= ChNb))
  {
    Ch--;
    OneRawData = RawData[Ch];
  }
  
  return(OneRawData);
}

uint16_t SrxlRxClass::width_us(uint8_t Ch)
{
  uint16_t Width_us = 1500;
  
  if((Ch >= 1) && (Ch <= ChNb))
  {
    Ch--;
    Width_us = Channel[Ch];
  }
  
  return(Width_us);
}

uint8_t SrxlRxClass::isSynchro(uint8_t SynchroClientIdx /*= 7*/)
{
  uint8_t Ret;
  
  Ret = !!(Synchro & RCUL_CLIENT_MASK(SynchroClientIdx));
  if(Ret) Synchro &= ~RCUL_CLIENT_MASK(SynchroClientIdx); /* Clear indicator for the Synchro client */
  
  return(Ret);
}

/* Rcul support */
uint8_t SrxlRxClass::RculIsSynchro(uint8_t ClientIdx /*= RCUL_DEFAULT_CLIENT_IDX*/)
{
  return(isSynchro(ClientIdx));  
}

uint16_t SrxlRxClass::RculGetWidth_us(uint8_t Ch)
{
  return(width_us(Ch));
}

void SrxlRxClass::RculSetWidth_us(uint16_t Width_us, uint8_t Ch /*= 255*/)
{
  Width_us = Width_us; /* To avoid a compilation warning */
  Ch = Ch;             /* To avoid a compilation warning */
}

static uint16_t crc16_CCITT(uint16_t crc, uint8_t value)
{
  uint8_t i;

  crc = crc ^ (int16_t)value << 8;

  for (i = 0; i < 8; i++)
  {
    if (crc & 0x8000)
    {
      crc = crc << 1 ^ 0x1021;
    } else
    {
      crc = crc << 1;
    }
  }
  return crc;
}
