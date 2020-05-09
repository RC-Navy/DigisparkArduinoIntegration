#include <RcBusRx.h>

enum {
           RCBUSRX_IDLE = 0,
/* SBUS */ RCBUSRX_SBUS_WAIT_FOR_0x0F,         RCBUSRX_SBUS_WAIT_FOR_END_OF_DATA,     RCBUSRX_SBUS_WAIT_FOR_0x00,
/* SRXL */ RCBUSRX_SRXL_WAIT_FOR_0xA1_OR_0xA2, RCBUSRX_SRXL_WAIT_FOR_CHANNELS,        RCBUSRX_SRXL_WAIT_FOR_CHECKSUM,
/* SUMD */ RCBUSRX_SUMD_WAIT_FOR_0xA8,         RCBUSRX_SRXL_WAIT_FOR_ST_0x01_OR_0x81, RCBUSRX_SRXL_WAIT_FOR_CH_NB,
           RCBUSRX_SUMD_WAIT_FOR_CHANNELS,     RCBUSRX_SUMD_WAIT_FOR_CHECKSUM,
/* IBUS */ RCBUSRX_IBUS_WAIT_FOR_LEN_0x20,     RCBUSRX_IBUS_WAIT_FOR_CMD_0x40,        RCBUSRX_IBUS_WAIT_FOR_CHANNELS,
           RCBUSRX_IBUS_WAIT_FOR_CHECKSUM,
/* JETI */ RCBUSRX_JETI_WAIT_FOR_0x3E,         RCBUSRX_JETI_WAIT_FOR_0x03,            RCBUSRX_JETI_WAIT_FOR_MSG_LEN,
           RCBUSRX_JETI_WAIT_FOR_PKT_ID,       RCBUSRX_JETI_WAIT_FOR_DATA_CH,         RCBUSRX_JETI_WAIT_FOR_DATA_LEN,
           RCBUSRX_JETI_WAIT_FOR_CHANNELS,     RCBUSRX_JETI_WAIT_FOR_CHECKSUM
     };

RcBusRxClass RcBusRx = RcBusRxClass();

#if (SBUS_RX_SRXL_SUPPORT == 1) || (SBUS_RX_SUMD_SUPPORT == 1)
static uint16_t crc16_CCITT(uint16_t crc, uint8_t value); // For SRXL and SUMD
#endif

#if (SBUS_RX_JETI_SUPPORT == 1)
static uint16_t Jeti_crc16_CCITT(uint16_t crc, uint8_t value); // For JETI
#endif

#define millis8()            (uint8_t)(millis() & 0x000000FF)
#define MIN_SLIENCE_TIME_MS  3 // We have to wait at least 2 ms of silence before listening for the protocol frame header

//#if !defined(HTONS(x))
#define HTONS(x)             __builtin_bswap16((uint16_t) (x))
//#endif

#define SBUS_BIT_PER_CH      11

/*************************************************************************
                              GLOBAL VARIABLES
*************************************************************************/
/* Constructor */
RcBusRxClass::RcBusRxClass()
{
  RxSerial = NULL;
  RxState  = RCBUSRX_IDLE; // SBUS by default: skip characters until a silence happened
  ChNb     = 0;            // No channel until a vlid fram is received
  Synchro  = 0x00;
  memset(Channel, 0, sizeof(Channel));
}

void RcBusRxClass::serialAttach(Stream *RxStream)
{
  RxSerial = RxStream;
}

void RcBusRxClass::setProto(uint8_t Proto)
{
  Info.Proto = Proto;
}

void RcBusRxClass::process(void)
{
  uint8_t  RxChar, Finished = 0;
  uint16_t *WordPtr, Word;
  
  if(RxSerial)
  {
    if(millis8() - StartMs > MIN_SLIENCE_TIME_MS)
    {
      StartMs = millis8();
      switch(Info.Proto)
      {
#if (SBUS_RX_SBUS_SUPPORT == 1)
        case RC_BUS_RX_SBUS:
        ChNb = SBUS_RX_CH_NB;
        RxState = RCBUSRX_SBUS_WAIT_FOR_0x0F;
        break;
#endif        
#if (SBUS_RX_SRXL_SUPPORT == 1)
        case RC_BUS_RX_SRXL:
        /* ChNb will be set when header received */
        RxState = RCBUSRX_SRXL_WAIT_FOR_0xA1_OR_0xA2;
        break;
#endif        
#if (SBUS_RX_SUMD_SUPPORT == 1)
        case RC_BUS_RX_SUMD:
        RxState = RCBUSRX_SUMD_WAIT_FOR_0xA8;
        break;
#endif        
#if (SBUS_RX_IBUS_SUPPORT == 1)
        case RC_BUS_RX_IBUS:
        ChNb = IBUS_RX_CH_NB;
        RxState = RCBUSRX_IBUS_WAIT_FOR_LEN_0x20;
        break;
#endif        
#if (SBUS_RX_JETI_SUPPORT == 1)
        case RC_BUS_RX_JETI:
        RxState = RCBUSRX_JETI_WAIT_FOR_0x3E;
        break;
#endif        
      }
    }
    while(RxSerial->available() > 0)
    {
      StartMs = millis8();
      RxChar = RxSerial->read();
      switch(RxState)
      {
#if (SBUS_RX_SBUS_SUPPORT == 1)
        /*****************/
        /* SBUS PROTOCOL */
        /*****************/
        case RCBUSRX_SBUS_WAIT_FOR_0x0F:
        if(RxChar == 0x0F)
        {
 //         StartMs = millis8(); /* Start of frame */
          RxIdx = SBUS_RX_DATA_NB - 1; // Trick: store from the end
          RxState = RCBUSRX_SBUS_WAIT_FOR_END_OF_DATA;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_SBUS_WAIT_FOR_END_OF_DATA:
        RawData[RxIdx] = RxChar;
        RxIdx--;
        if(RxIdx == 255)
        {
          /* Check next byte is 0x00 */
          RxState = RCBUSRX_SBUS_WAIT_FOR_0x00;
        }
        break;

        case RCBUSRX_SBUS_WAIT_FOR_0x00:
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
        RxState = RCBUSRX_SBUS_WAIT_FOR_0x0F;
        break;
#endif
#if (SBUS_RX_SRXL_SUPPORT == 1)
        /*****************/
        /* SRXL PROTOCOL */
        /*****************/
        case RCBUSRX_SRXL_WAIT_FOR_0xA1_OR_0xA2:
        if((RxChar == 0xA1) || (RxChar == 0xA2))
        {
          ChNb = (RxChar == 0xA1)? SRXL_RX_A1_CH_NB: SRXL_RX_A2_CH_NB;
          RxIdx = 0;
          ComputedCrc = 0;
          ComputedCrc = crc16_CCITT(ComputedCrc, RxChar); // 0xA1 or 0xA2 headers are not stored to save RAM
          RxState = RCBUSRX_SRXL_WAIT_FOR_CHANNELS;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_SRXL_WAIT_FOR_CHANNELS:
        RawData[RxIdx++] = RxChar;
        ComputedCrc = crc16_CCITT(ComputedCrc, RxChar);
        if(RxIdx >= (2 * ChNb))
        {
          RxState = RCBUSRX_SRXL_WAIT_FOR_CHECKSUM;
        }
        break;

        case RCBUSRX_SRXL_WAIT_FOR_CHECKSUM:
        RxIdx++; /* Rx Checksum is not stored: just increment index */
        ComputedCrc = crc16_CCITT(ComputedCrc, RxChar);
        if(RxIdx >= ((2 * ChNb) + 2))
        {
          if(!ComputedCrc)
          {
            /* Update channels from raw data */
            for(uint8_t Idx = 0; Idx < ChNb; Idx++)
            {
              WordPtr = (uint16_t *)&RawData[Idx * 2];
              Word = HTONS(*WordPtr);
              Channel[Idx] = map(Word, 0, 0x0FFF, 800, 2200);
            }
            Synchro  = 0xFF;
            Finished = 1;
          }
          else RxState = RCBUSRX_IDLE;
        }
        break;
#endif
#if (SBUS_RX_SUMD_SUPPORT == 1)        
        case RCBUSRX_SUMD_WAIT_FOR_0xA8:
        if(RxChar == 0xA8)
        {
          ComputedCrc = 0;
          ComputedCrc = crc16_CCITT(ComputedCrc, RxChar); // 0xA8 header is not stored to save RAM
          RxState = RCBUSRX_SRXL_WAIT_FOR_ST_0x01_OR_0x81;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_SRXL_WAIT_FOR_ST_0x01_OR_0x81:
        if((RxChar == 0x01) || (RxChar == 0x81))
        {
          ComputedCrc = crc16_CCITT(ComputedCrc, RxChar);
          Info.FailSafe = (RxChar == 0x81);
          RxState = RCBUSRX_SRXL_WAIT_FOR_CH_NB;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_SRXL_WAIT_FOR_CH_NB:
        if((RxChar >= 2) && (RxChar <= 16))
        {
          ComputedCrc = crc16_CCITT(ComputedCrc, RxChar);
          ChNb = RxChar;
          RxIdx = 0;
          RxState = RCBUSRX_SUMD_WAIT_FOR_CHANNELS;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_SUMD_WAIT_FOR_CHANNELS:
        RawData[RxIdx++] = RxChar;
        ComputedCrc = crc16_CCITT(ComputedCrc, RxChar);
        if(RxIdx >= (2 * ChNb))
        {
          RxState = RCBUSRX_SUMD_WAIT_FOR_CHECKSUM;
        }
        break;

        case RCBUSRX_SUMD_WAIT_FOR_CHECKSUM:
        RxIdx++; /* Rx Checksum is not stored: just increment index */
        ComputedCrc = crc16_CCITT(ComputedCrc, RxChar);
        if(RxIdx >= ((2 * ChNb) + 2))
        {
          if(!ComputedCrc)
          {
            /* Update channels from raw data */
            for(uint8_t Idx = 0; Idx < ChNb; Idx++)
            {
              WordPtr = (uint16_t *)&RawData[Idx * 2];
              Word = HTONS(*WordPtr);
              Channel[Idx] = map(Word, 0x1C20, 0x41A0, 900, 2100);
            }
            Synchro  = 0xFF;
            Finished = 1;
          }
          else RxState = RCBUSRX_IDLE;
        }
        break;
#endif
#if (SBUS_RX_IBUS_SUPPORT == 1)        
        /*****************/
        /* IBUS PROTOCOL */
        /*****************/
        case RCBUSRX_IBUS_WAIT_FOR_LEN_0x20:
        if(RxChar == 0x20)
        {
          ComputedCrc = 0xFFFF - 0x20;
          RxState = RCBUSRX_IBUS_WAIT_FOR_CMD_0x40;
        }
        break;

        case RCBUSRX_IBUS_WAIT_FOR_CMD_0x40:
        if(RxChar == 0x40)
        {
          ComputedCrc -= 0x40;
          RxIdx = 0;
          RxState = RCBUSRX_IBUS_WAIT_FOR_CHANNELS;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_IBUS_WAIT_FOR_CHANNELS:
        RawData[RxIdx++] = RxChar;
        ComputedCrc -= RxChar;
        if(RxIdx >= (2 * ChNb))
        {
          RxState = RCBUSRX_IBUS_WAIT_FOR_CHECKSUM;
        }
        break;

        case RCBUSRX_IBUS_WAIT_FOR_CHECKSUM:
        RawData[RxIdx] = RxChar;
        RxIdx++; /* Rx Checksum is not stored: just increment index */
        if(RxIdx >= ((2 * ChNb) + 2))
        {
          if(ComputedCrc == (uint16_t)((RxChar << 8) + RawData[(2 * ChNb)])) // If Checksum = 0, frame is OK
          {
            /* Update channels from raw data */
            for(uint8_t Idx = 0; Idx < ChNb; Idx++)
            {
              WordPtr = (uint16_t *)&RawData[Idx * 2];
              Word = *WordPtr;
              Channel[Idx] = Word; // With IBUS, RawData is directly Channel in us
            }
            Synchro  = 0xFF;
            Finished = 1;
          }
          RxState = RCBUSRX_IDLE;
        }
        break;
#endif
#if (SBUS_RX_JETI_SUPPORT == 1)
        case RCBUSRX_JETI_WAIT_FOR_0x3E:
        if(RxChar == 0x3E)
        {
          ComputedCrc = 0;
          ComputedCrc = Jeti_crc16_CCITT(ComputedCrc, RxChar); // 0x3E header is not stored to save RAM
          RxState = RCBUSRX_JETI_WAIT_FOR_0x03;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_JETI_WAIT_FOR_0x03:
        if(RxChar == 0x03)
        {
          ComputedCrc = Jeti_crc16_CCITT(ComputedCrc, RxChar); // 0x03 header is not stored to save RAM
          RxState = RCBUSRX_JETI_WAIT_FOR_MSG_LEN;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_JETI_WAIT_FOR_MSG_LEN:
        ComputedCrc = Jeti_crc16_CCITT(ComputedCrc, RxChar); // not stored to save RAM
        RxState = RCBUSRX_JETI_WAIT_FOR_PKT_ID;
        break;

        case RCBUSRX_JETI_WAIT_FOR_PKT_ID:
        ComputedCrc = Jeti_crc16_CCITT(ComputedCrc, RxChar); // not stored to save RAM
        RxState = RCBUSRX_JETI_WAIT_FOR_DATA_CH;
        break;

        case RCBUSRX_JETI_WAIT_FOR_DATA_CH:
        if(RxChar == 0x31) // SHALL be 0x31 (Data ID for channels)
        {
          ComputedCrc = Jeti_crc16_CCITT(ComputedCrc, RxChar); // 0x31 header is not stored to save RAM
          RxState = RCBUSRX_JETI_WAIT_FOR_DATA_LEN;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_JETI_WAIT_FOR_DATA_LEN:
        if((RxChar >= (4 * 2)) && (RxChar <= (16 * 2))) // 4 to 16 channels?
        {
          ChNb  = RxChar / 2;
          RxIdx = 0;
          ComputedCrc = Jeti_crc16_CCITT(ComputedCrc, RxChar); // not stored to save RAM
          RxState = RCBUSRX_JETI_WAIT_FOR_CHANNELS;
        }
        else RxState = RCBUSRX_IDLE;
        break;

        case RCBUSRX_JETI_WAIT_FOR_CHANNELS:
        RawData[RxIdx++] = RxChar;
        ComputedCrc = Jeti_crc16_CCITT(ComputedCrc, RxChar);
        if(RxIdx >= (2 * ChNb))
        {
          RxState = RCBUSRX_JETI_WAIT_FOR_CHECKSUM;
        }
        break;

        case RCBUSRX_JETI_WAIT_FOR_CHECKSUM:
        RawData[RxIdx] = RxChar;
        RxIdx++; /* Rx Checksum is not stored: just increment index */
        if(RxIdx >= ((2 * ChNb) + 2))
        {
          if(ComputedCrc == (uint16_t)((RxChar << 8) + RawData[(2 * ChNb)])) // If Checksum = 0, frame is OK
          {
            /* Update channels from raw data */
            for(uint8_t Idx = 0; Idx < ChNb; Idx++)
            {
              WordPtr = (uint16_t *)&RawData[Idx * 2];
              Word = *WordPtr;
              Channel[Idx] = Word >> 3; // With JETI, RawData is Channel in us x 8
            }
            Synchro  = 0xFF;
            Finished = 1;
          }
          RxState = RCBUSRX_IDLE;
        }
        break;
#endif
      }
      if(Finished) break;
    }
  }
}

uint8_t RcBusRxClass::channelNb(void)
{
  return(ChNb);
}

uint16_t RcBusRxClass::rawData(uint8_t Ch)
{
  uint16_t *WordPtr, OneRawData = 0x0800; /* Corresponds to 1500 us for SRXL */
  
  if((Ch >= 1) && (Ch <= ChNb))
  {
    Ch--; // To become an index
    switch(Info.Proto)
    {
      case RC_BUS_RX_SBUS:
      case RC_BUS_RX_IBUS:
      OneRawData = Channel[Ch];
      break;
      
      case RC_BUS_RX_SRXL:
      case RC_BUS_RX_SUMD:
      WordPtr = (uint16_t *)&RawData[Ch * 2];
      OneRawData = HTONS(*WordPtr);
      break;
      
      case RC_BUS_RX_JETI:
      WordPtr = (uint16_t *)&RawData[Ch * 2];
      OneRawData = *WordPtr;
      break;
    }
  }
  return(OneRawData);
}

uint16_t RcBusRxClass::width_us(uint8_t Ch)
{
  uint16_t Width_us = 1500;
  
  if((Ch >= 1) && (Ch <= SBUS_RX_CH_NB))
  {
    Ch--; 
    switch(Info.Proto)
    {
      case RC_BUS_RX_SBUS:
      Width_us = map(Channel[Ch], 0, 2047, 880, 2160);
      break;

      case RC_BUS_RX_SRXL:
      case RC_BUS_RX_SUMD:
      case RC_BUS_RX_IBUS:
      case RC_BUS_RX_JETI:
      Width_us = Channel[Ch];
      break;
      
      default:
      break;
    }
  }
  return(Width_us);
}

uint8_t RcBusRxClass::isSynchro(uint8_t SynchroClientIdx /*= 7*/)
{
  uint8_t Ret;
  
  Ret = !!(Synchro & RCUL_CLIENT_MASK(SynchroClientIdx));
  if(Ret) Synchro &= ~RCUL_CLIENT_MASK(SynchroClientIdx); /* Clear indicator for the Synchro client */
  
  return(Ret);
}

uint8_t RcBusRxClass::flags(uint8_t FlagId)
{
  uint8_t Flag = 0;
  switch(Info.Proto)
  {
#if (SBUS_RX_SBUS_SUPPORT == 1)
    case RC_BUS_RX_SBUS:
    Flag = !!(RawData[0] & FlagId);
    break;
#endif
#if (SBUS_RX_SUMD_SUPPORT == 1)
    case RC_BUS_RX_SUMD:
    FlagId = FlagId; // To avoid a compilation warning
    Flag = Info.FailSafe;
    break;
#endif
    default:
    break;
  }
  return(Flag);
}

/* Rcul support */
uint8_t RcBusRxClass::RculIsSynchro(uint8_t ClientIdx /*= RCUL_DEFAULT_CLIENT_IDX*/)
{
  return(isSynchro(ClientIdx));  
}

uint16_t RcBusRxClass::RculGetWidth_us(uint8_t Ch)
{
  return(width_us(Ch));
}

void RcBusRxClass::RculSetWidth_us(uint16_t Width_us, uint8_t Ch /*= 255*/)
{
  Width_us = Width_us; /* To avoid a compilation warning */
  Ch = Ch;             /* To avoid a compilation warning */
}

/* Only needed by SBUS */
void RcBusRxClass::updateChannels(void)
{
  uint8_t  DataIdx = SBUS_RX_DATA_NB - 1, DataBitIdx = 0, ChIdx = 0, ChBitIdx = 0;

  for(uint8_t GlobBitIdx = 0; GlobBitIdx < (SBUS_RX_CH_NB * SBUS_BIT_PER_CH); GlobBitIdx++)
  {
    bitWrite(Channel[ChIdx], ChBitIdx, bitRead(RawData[DataIdx], DataBitIdx));
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

#if (SBUS_RX_SRXL_SUPPORT == 1) || (SBUS_RX_SUMD_SUPPORT == 1)
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
#endif

#if (SBUS_RX_JETI_SUPPORT == 1)
static uint16_t Jeti_crc16_CCITT(uint16_t crc, uint8_t data)
{
  uint16_t ret_val;
  data ^= (uint8_t)(crc) & (uint8_t)(0xFF);
  data ^= data << 4;
  ret_val = ((((uint16_t)data << 8) | ((crc & 0xFF00) >> 8))
          ^ (uint8_t)(data >> 4)
          ^ ((uint16_t)data << 3));
  return ret_val;
}
#endif
