#include <RcBusRx.h>

const uint8_t SBusVector[] PROGMEM = {
  /* 00: Header */0x0f,
  /* 01: CH   */  0x01,0x04,
  /* 03: CH   */  0x20,0x00,
  /* 05: CH   */  0xff,0x07,
  /* 07: CH   */  0x40,0x00,
  /* 09: CH   */  0x02,0x10,
  /* 11: CH   */  0x80,0x2c,
  /* 13: CH   */  0x64,0x21,
  /* 15: CH   */  0x0b,0x59,
  /* 17: CH   */  0x08,0x40,
  /* 19: CH   */  0x00,0x02,
  /* 21: CH   */  0x10,0x80,
  /* 23: CH   */  0x00,
  /* 24:Trailer */0x00};

const uint8_t SrxlVector[] PROGMEM = {
  /* 00: Header */ 0xA1,
  /* 01: CH01 */   0x08, 0x00,
  /* 03: CH02 */   0x07, 0xFF,
  /* 05: CH03 */   0x08, 0x00,
  /* 07: CH04 */   0x01, 0xB7,
  /* 09: CH05 */   0x01, 0xB7,
  /* 11: CH06 */   0x01, 0x23,
  /* 13: CH07 */   0x01, 0xB7,
  /* 15: CH08 */   0x01, 0xB7,
  /* 17: CH09 */   0x01, 0xB7,
  /* 19: CH10 */   0x01, 0xB7,
  /* 21: CH11 */   0x01, 0xB7,
  /* 23: CH12 */   0x01, 0xB7,
  /* 25: CRC  */   0xDE, 0xAC
                };

const uint8_t SumdVector[] PROGMEM = {
  /* 00: Header */ 0xA8,
  /* 01: ST     */ 0x01,
  /* 02: CH  Nb */ 0x08,
  /* 03: CH01 */   0x3B, 0x60,
  /* 05: CH02 */   0x2E, 0xD8,
  /* 07: CH03 */   0x2E, 0xD8,
  /* 09: CH04 */   0x2E, 0xD0,
  /* 11: CH05 */   0x2E, 0xE0,
  /* 13: CH06 */   0x2E, 0xE0,
  /* 15: CH07 */   0x2E, 0xE0,
  /* 17: CH08 */   0x2E, 0xE0,
  /* 19: CRC  */   0xD8, 0xB3
};

const uint8_t iBusVector[] PROGMEM = {
 /* 00: Length */  0x20,
 /* 01: Cmd code*/ 0x40,
 /* 02: CH01   */  0xDB, 0x05,
 /* 04: CH02   */  0xDC, 0x05,
 /* 06: CH03   */  0x54, 0x05,
 /* 08: CH04   */  0xDC, 0x05,
 /* 10: CH05   */  0xE8, 0x03,
 /* 12: CH06   */  0xD0, 0x07,
 /* 14: CH07   */  0xD2, 0x05,
 /* 16: CH08   */  0xE8, 0x03,
 /* 18: CH09   */  0xDC, 0x05,
 /* 20: CH10   */  0xDC, 0x05,
 /* 22: CH11   */  0xDC, 0x05,
 /* 24: CH12   */  0xDC, 0x05,
 /* 26: CH13   */  0xDC, 0x05,
 /* 28: CH14   */  0xDC, 0x05,
 /* 30: CRC    */  0xDA, 0xF3
};

const uint8_t JetiVector[] PROGMEM = {
 /* 00: Head 1 */  0x3E,
 /* 01: Head 2 */  0x03,
 /* 02: Msg Len*/  0x28,
 /* 03: Pkt ID */  0x06,
 /* 04: Dat Ch */  0x31,
 /* 05: Blk Len*/  0x20,
 /* 06: CH01   */  0x82, 0x1F,
 /* 08: CH02   */  0x82, 0x1F,
 /* 10: CH03   */  0x82, 0x1F,
 /* 12: CH04   */  0x82, 0x1F,
 /* 14: CH05   */  0x82, 0x1F,
 /* 16: CH06   */  0x82, 0x1F,
 /* 18: CH07   */  0x82, 0x1F,
 /* 20: CH08   */  0x82, 0x1F,
 /* 22: CH09   */  0x82, 0x1F,
 /* 24: CH10   */  0x82, 0x1F,
 /* 26: CH11   */  0x82, 0x1F,
 /* 28: CH12   */  0x82, 0x1F,
 /* 30: CH13   */  0x82, 0x1F,
 /* 32: CH14   */  0x82, 0x1F,
 /* 34: CH15   */  0x82, 0x1F,
 /* 36: CH16   */  0x82, 0x1F,
 /* 38: CRC    */  0x4F, 0xE2
};

typedef struct{
  uint8_t *Frame;
  uint8_t  FrameLen;
}TestVectorSt_t;

TestVectorSt_t TestVector[] = {{SBusVector, sizeof(SBusVector)}, {SrxlVector, sizeof(SrxlVector)}, {SumdVector, sizeof(SumdVector)}, {iBusVector, sizeof(iBusVector)}, {JetiVector, sizeof(JetiVector)}};

char    Proto = 's'; // s -> SBus, x -> SRXL, d -> SumD, i -> IBUS, j -> JETI
char    ProtoName[10];
uint8_t TestVectIdx = 0;

void setup()
{
  while(!Serial);
  Serial.begin(115200); /* For serial console output */
  ConfigForProto(Proto);
  RcBusRx.serialAttach(&Serial1); /* Then, attach the SBus receiver to this Serial1 */
  Serial.println();
  Serial.println(F("   ***   RcBusTx demo ***"));
  Serial.println();
  Serial.println(F("1) Connect TX1 to RX1 with a simple wire or with a 1K resistor"));
  Serial.println();
  Serial.println(F("2) In the serial console, type the following letters + enter to test each supported serial protocol:"));
  Serial.println(F("- 's' -> SBUS"));
  Serial.println(F("- 'x' -> SRXL"));
  Serial.println(F("- 'd' -> SUMD"));
  Serial.println(F("- 'i' -> IBUS"));
  Serial.println(F("- 'j' -> JETI"));
  delay(3000);
}

void loop()
{
  static uint32_t StartMs = millis();
  static uint8_t TxInProgress = 0;
  char Str[10];
  char RxChar;
  
  if(Serial.available())
  {
    RxChar = Serial.read();
    ConfigForProto(RxChar);
  }
  
  if((millis() - StartMs) >= 1000UL)
  {
    StartMs = millis();
    Serial.print(F("\nSend "));Serial.print(ProtoName);Serial.print(F(" test frame: "));
    for(uint8_t Idx = 0; Idx < TestVector[TestVectIdx].FrameLen; Idx++)
    {
      sprintf(Str, "0x%02X ", (uint8_t)pgm_read_byte(&TestVector[TestVectIdx].Frame[Idx]));
      Serial.print(Str);
    }
    Serial.println();
    for(uint8_t Idx = 0; Idx < TestVector[TestVectIdx].FrameLen; Idx++)
    {
      Serial1.write((uint8_t)pgm_read_byte(&TestVector[TestVectIdx].Frame[Idx]));
    }
    TxInProgress = 1;
  }
  RcBusRx.process(); /* Don't forget to call the SBusRx.process()! */
  
  if(RcBusRx.isSynchro()) /* One SBUS frame just arrived */
  {
    Serial.print(F("\nReceived "));Serial.print(ProtoName);Serial.println(F(" frame:"));
    /* Display SBUS channels and flags in the serial console */
    for(uint8_t Ch = 1; Ch <= RcBusRx.channelNb(); Ch++)
    {
      Serial.print(F("Ch["));Serial.print(Ch);Serial.print(F("]="));Serial.print(RcBusRx.width_us(Ch));Serial.print(F(" Raw=0x"));Serial.println(RcBusRx.rawData(Ch), HEX);
    }
    if(Proto == 's')
    {
      Serial.print(F("Ch17="));    Serial.println(RcBusRx.flags(SBUS_RX_CH17)); /* Digital Channel#17 */
      Serial.print(F("Ch18="));    Serial.println(RcBusRx.flags(SBUS_RX_CH18)); /* Digital Channel#18 */
      Serial.print(F("FrmLost=")); Serial.println(RcBusRx.flags(SBUS_RX_FRAME_LOST)); /* Switch off the Transmitter to check this */
    }
    if((Proto == 's') || (Proto == 'd'))
    {
      Serial.print(F("FailSafe="));Serial.println(RcBusRx.flags(SBUS_RX_FAILSAFE));   /* Switch off the Transmitter to check this */
    }
    TxInProgress = 0;
  }
  else
  {
    if(((millis() - StartMs) >= 500UL) && TxInProgress)
    {
      StartMs = millis();
      Serial.print(F("Timeout, no response!"));
      TxInProgress = 0;
    }
  }
}

void ConfigForProto(char RxProto)
{
  switch(RxProto)
  {
    case 's':
    Serial1.begin(SBUS_RX_SERIAL_CFG);
    RcBusRx.setProto(RC_BUS_RX_SBUS);
    strcpy_P(ProtoName, PSTR("SBUS"));
    TestVectIdx = 0;
    Proto = RxProto;
    break;

    case 'x':
    Serial1.begin(SRXL_RX_SERIAL_CFG);
    RcBusRx.setProto(RC_BUS_RX_SRXL);
    strcpy_P(ProtoName, PSTR("SRXL"));
    TestVectIdx = 1;
    Proto = RxProto;
    break;
    
    case 'd':
    Serial1.begin(SUMD_RX_SERIAL_CFG);
    RcBusRx.setProto(RC_BUS_RX_SUMD);
    strcpy_P(ProtoName, PSTR("SUMD"));
    TestVectIdx = 2;
    Proto = RxProto;
    break;

    case 'i':
    Serial1.begin(IBUS_RX_SERIAL_CFG);
    RcBusRx.setProto(RC_BUS_RX_IBUS);
    strcpy_P(ProtoName, PSTR("IBUS"));
    TestVectIdx = 3;
    Proto = RxProto;
    break;

    case 'j':
    Serial1.begin(JETI_RX_SERIAL_CFG);
    RcBusRx.setProto(RC_BUS_RX_JETI);
    strcpy_P(ProtoName, PSTR("JETI"));
    TestVectIdx = 4;
    Proto = RxProto;
    break;

  }
}
