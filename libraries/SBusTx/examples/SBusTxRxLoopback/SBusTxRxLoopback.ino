/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  \_\  \____/    |_|  \__| |_|  |_|   \__/      |_| 2016

                http://p.loussouarn.free.fr
English:
=======
This sketch demonstrates how to use the <SBusTx> and <SBusRx> libraries.
This sketch requires 2 Serial ports on the Arduino, so it works on Arduino Leonardo and Arduino MEGA.
The aim is to change the values of the 16 RC Channels and the SBUS flags in the serial console and to
check the changes in the SBus Rx part.
Just tie the Serial1 Tx to the Serial1 Rx pin of the Serial1 UART on the arduino.

Francais:
========
Ce sketch demontre comment utiliser les bibliotheques <SBusTx> et <SBusRx>.
Ce sketch necessite 2 ports serie sur l'Arduino, il fonctionne donc sur arduino Leonardo et Arduino MEGA.
Le but est de changer les valeurs des 16 voies RC et des indicateurs SBUS dans la console serie et de
verifier les changements dans la partie SBus Rx.
Connecter simplement le signal Serial1 Tx au signal Serial1 Rx de l'UART Serial1 de l'arduino.

Wiring/Cablage:
==============
               .----------------------.           .------------------.  
               |    Leonardo/MEGA     |           |       PC         |
           .---+ Serial1 Tx           |           | (Serial Console) |
           |   |                   USB+-----------+USB               |
           '-->+ Serial1 Rx           | USB cable |                  |
               |                      |           |                  |
               '----------------------'           '------------------'

*/
#include <SBusTx.h>
#include <SBusRx.h>


#define CARRIAGE_RETURN    0x0D /* '\r' = 0x0D (code ASCII) */
#define RUB_OUT            0x08

#define TM_MSG_MAX_LENGTH  20 /* Message le plus long */

static char     TmMessage[TM_MSG_MAX_LENGTH + 1];/* + 1 pour fin de chaine */
static uint32_t StartMs = millis();
static uint8_t  SBusRqst = 0;

void setup()
{
  while(!Serial);
  Serial.begin(115200);
  Serial1.begin(100000, SERIAL_8E2);

  SBusTx.serialAttach(&Serial1);
  SBusRx.serialAttach(&Serial1);

  delay(1000);
  Serial.println(F("Test of SBusTx and SBusRx libraries in loopback mode: (Tx tied to Rx on Serial1 UART)\n"));
  Serial.println(F("Usage in a serial console at 115200 bds:"));
  Serial.println(F("Cxx=Value (in us) with xx = channel number (01 to 18: 17 and 18 are digital channels)"));
  Serial.println(F("ex: C01=1500 or C17=1 (C17 and C18 are digital channels: possible values are 0 or 1)"));
  Serial.println(F("Dxx=Value (raw value) with xx = channel number (01 to 16)"));
  Serial.println(F("ex: D16=1024 (Raw data value rather than value in us)"));
  Serial.println(F("F=Value (Set or clear the FrameLost flag: value=0 or 1)"));
  Serial.println(F("S=Value (Set or clear the FailSafe  flag: value=0 or 1)"));
  Serial.println(F("Cxx? Dxx? F? and S? give the current values of the last received SBus frame\n"));
}

void loop()
{
  
  if(TmMessageAvailable() >= 0)
  {
    InterpreteTmAndExecute();
  }
  
  SBusRx.process(); /* Don't forget to call the SBusRx.process()! */
  
  if((millis() - StartMs >= 10UL) && SBusRqst)
  {
    if(SBusRx.isSynchro()) /* One SBUS frame just arrived */
    {
      Serial.println(F("SBUS data received with SBusRx library:"));
      /* Display SBUS channels and flags in the serial console */
      for(uint8_t Ch = 1; Ch <= SBUS_RX_CH_NB; Ch++)
      {
        Serial.print(F("Ch["));Serial.print(Ch);Serial.print(F("]="));Serial.print(SBusRx.width_us(Ch));Serial.print(F(" Raw="));Serial.println(SBusRx.rawData(Ch));
      }
      Serial.print(F("Ch17="));    Serial.println(SBusRx.flags(SBUS_RX_CH17)); /* Digital Channel#17 */
      Serial.print(F("Ch18="));    Serial.println(SBusRx.flags(SBUS_RX_CH18)); /* Digital Channel#18 */
      Serial.print(F("FrmLost ="));Serial.println(SBusRx.flags(SBUS_RX_FRAME_LOST)); /* Switch off the Transmitter to check this */
      Serial.print(F("FailSafe="));Serial.println(SBusRx.flags(SBUS_RX_FAILSAFE));   /* Switch off the Transmitter to check this */
    }
    SBusRqst = 0;
  }

}

//==============================================================================================
static char TmMessageAvailable()
{
  char Ret=-1;
  char RxChar;
  static uint8_t Idx=0;

  if(Serial.available() > 0)
  {
    RxChar=Serial.read();
    switch(RxChar)
    {
      case CARRIAGE_RETURN: /* Si retour chariot: fin de message */
      TmMessage[Idx]=0;/* Remplace CR character par fin de chaine */
      Ret=Idx;
      Idx=0; /* Re-positionne index pour prochain message */
      break;
            
      case RUB_OUT:
      if(Idx) Idx--;
      break;
            
      default:
      if(Idx < TM_MSG_MAX_LENGTH)
      {
        TmMessage[Idx]=RxChar;
        Idx++;
      }
      else Idx=0; /* Re-positionne index pour prochain message */
      break;
    }
  }
  return(Ret); 
}
/*************************************************/
/*          COMMAND FOR INTERPRETOR              */
/*************************************************/
#define CHANNEL_CMD            'C'
#define DATA_CMD               'D'

#define FRAME_LOST_CMD         'F'
#define FAILSAFE_CMD           'S'

/* MACROs FOR INTERPRETOR */
#define COMMAND    (TmMessage[0])
#define ACTION(n)  (TmMessage[n])
#define ARG(n)     (TmMessage[n])
#define REQUEST    '?'
#define ORDER      '='
enum {ACTION_NONE=0, ACTION_ANSWER_WITH_REPONSE, ACTION_ANSWER_UNKNOWN_COMMAND, ACTION_ANSWER_OUT_OF_RANGE, ACTION_NOT_POSSIBLE_IN_THIS_CONTEXT, ACTION_ANSWER_ERROR};

static void InterpreteTmAndExecute(void)
{
  char Action = ACTION_ANSWER_WITH_REPONSE;
  uint16_t Ch, Value;
  switch(COMMAND)
  {
    
    case CHANNEL_CMD:
    Ch = atoi(&ACTION(1));
    if(Ch >= 1 && Ch <= (SBUS_TX_CH_NB + 2)) // +2 for digital channel 17 & 18
    {
      switch(ACTION(3))
      {
        case REQUEST:
        ACTION(3) = '=';
        if(Ch <= SBUS_TX_CH_NB) itoa(SBusRx.width_us(Ch), &ARG(4), 10);
        else                    itoa(SBusRx.flags((Ch - 17) + SBUS_RX_CH17), &ARG(4), 10);
        break;
        
        case ORDER:
        Value = atoi(&ARG(4));
        if(Ch <= SBUS_TX_CH_NB) SBusTx.width_us(Ch, Value);
        else                    SBusTx.flags((Ch - 17) + SBUS_TX_CH17, (uint8_t)Value);
        ACTION(3) = 0;
        SBusRqst = 1;
        break;
        
        default:
        Action = ACTION_ANSWER_UNKNOWN_COMMAND;
        break;
      }
    }
    else Action = ACTION_ANSWER_OUT_OF_RANGE;
    break;
    
    case DATA_CMD:
    Ch = atoi(&ACTION(1));
    if(Ch >= 1 && Ch <= SBUS_TX_CH_NB)
    {
      switch(ACTION(3))
      {
        case REQUEST:
        ACTION(3) = '=';
        itoa(SBusRx.rawData(Ch), &ARG(4), 10);
        break;
        
        case ORDER:
        Value = atoi(&ARG(4));
        SBusTx.rawData(Ch, Value);
        ACTION(3) = 0;
        SBusRqst = 1;
        break;
        
        default:
        Action = ACTION_ANSWER_UNKNOWN_COMMAND;
        break;
      }
    }
    else Action = ACTION_ANSWER_OUT_OF_RANGE;
    break;

    case FRAME_LOST_CMD:
    switch(ACTION(1))
    {
      case REQUEST:
      ACTION(1) = '=';
      ACTION(2) = '0' + SBusRx.flags(SBUS_TX_FRAME_LOST);
      ACTION(3) = 0;
      break;
      
      case ORDER:
      Value = atoi(&ARG(2));
      SBusTx.flags(SBUS_TX_FRAME_LOST, !!Value);
      ACTION(1) = 0;
      SBusRqst = 1;
      break;
      
      default:
      Action = ACTION_ANSWER_UNKNOWN_COMMAND;
      break;
    }
    break;

    case FAILSAFE_CMD:
    switch(ACTION(1))
    {
      case REQUEST:
      ACTION(1) = '=';
      ACTION(2) = '0' + SBusRx.flags(SBUS_RX_FAILSAFE);
      ACTION(3) = 0;
      break;
      
      case ORDER:
      Value = atoi(&ARG(2));
      SBusTx.flags(SBUS_TX_FAILSAFE, !!Value);
      ACTION(1) = 0;
      SBusRqst = 1;
      break;
      
      default:
      Action = ACTION_ANSWER_UNKNOWN_COMMAND;
      break;
    }
    break;

  }

  switch(Action)
  {
    case ACTION_NONE: /* Keep quiet */
    break;
    
    case ACTION_ANSWER_WITH_REPONSE:
    strcat(TmMessage, "\r"); /* Just to have a carriage return */
    if(SBusRqst)
    {
      SBusTx.sendChannels();
      StartMs = millis();
    }
    break;

    case ACTION_ANSWER_UNKNOWN_COMMAND:
    strcpy_P(TmMessage, PSTR("UKNWN CMD\r"));
    break;
    
    case ACTION_ANSWER_OUT_OF_RANGE:
    strcpy_P(TmMessage, PSTR("OOR\r"));
    break;

    case ACTION_NOT_POSSIBLE_IN_THIS_CONTEXT:
    strcpy_P(TmMessage, PSTR("CTX ERR\r"));
    break;
    
    case ACTION_ANSWER_ERROR:
    strcpy_P(TmMessage, PSTR("ERR\r"));
    break;
  }
  if(Action != ACTION_NONE)
  {
    Serial.println(TmMessage);
    if(SBusRqst) Serial.println(F("SBUS data sent with SBusTx library..."));
  }

}

