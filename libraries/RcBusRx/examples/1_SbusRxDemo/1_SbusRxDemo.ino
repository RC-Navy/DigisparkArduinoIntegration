/*
   _____     ____      __    _    ____    _    _   _     _ 
  |  __ \   / __ \    |  \  | |  / __ \  | |  | | | |   | |
  | |__| | | /  \_|   | . \ | | / /  \ \ | |  | |  \ \ / /
  |  _  /  | |   _    | |\ \| | | |__| | | |  | |   \ ' /
  | | \ \  | \__/ |   | | \ ' | |  __  |  \ \/ /     | |
  |_|  \_\  \____/    |_|  \__| |_|  |_|   \__/      |_| 2016-2020

                http://p.loussouarn.free.fr
English:
=======
This sketch demonstrates how to use the <RcBusRx> library when it is configured for SBUS.
This sketch requires 2 Serial ports on the Arduino, so it works on Arduino Leonardo and Arduino MEGA.
The aim is to display on the Serial Console, the values of the n RC Channels and the flags (Ch17, Ch18, FrmLost, FailSafe).
Just connect the Receiver SBUS signal to the Rx pin of the Serial1 UART on the arduino.
Attention: a serial inverseur is often needed!

Francais:
========
Ce sketch demontre comment utiliser la bibliotheque <RcBusRx> quand elle est configuree en SBUS.
Ce sketch necessite 2 ports serie sur l'Arduino, il fonctionne donc sur arduino Leonardo et Arduino MEGA.
Le but est d'afficher dans la console serie, les valeurs des n voies RC et les indicateurs ((Ch17, Ch18, FrmLost, FailSafe)).
Connecter simplement le signal SBus du recepteur a la broche Rx de l'UART Serial1 de l'arduino.
Attention: un inverseur serie est souvent necessaire!

Wiring/Cablage:
==============
 .---------------.    Serial    .----------------------.           .------------------.  
 |  RC Receiver  |   Inverter   |    Leonardo/MEGA     |           |       PC         |
 |               |     |\       |                      |           |                  | 
 |    SBus Signal+-----| o----->+ Serial1 Rx           |           | (Serial Console) |
 |               |     |/       |                   USB+-----------+USB               |
 |            Gnd+--------------+ Gnd                  | USB cable |                  |
 |               |              |                      |           |                  |
 '---------------'              '----------------------'           '------------------'

*/
#include <RcBusRx.h>

void setup()
{
  while(!Serial);
  Serial.begin(115200); /* For serial console output */
  Serial.println(F("RcBusRx configured in SBUS receiver (/!\\ A serial inverter may be needed /!\\)\n"));
  
  Serial1.begin(SBUS_RX_SERIAL_CFG); /* Choose your serial and select xxxx__RX_SERIAL_CFG (list in RcBusRx.h) where xxxx is the protocol */
  
  RcBusRx.serialAttach(&Serial1);    /* Then, attach the SBus receiver to this Serial1 */
  RcBusRx.setProto(RC_BUS_RX_SBUS);  /* Select here the serial protocol */
}

#define DISPLAY_EVERY_SERIAL_FRAME_NB   40 // To not flood the serial console!

void loop()
{
  static uint8_t SerialFrameCnt = 0;
  
  RcBusRx.process(); /* Don't forget to call the RcBusRx.process()! */
  
  if(RcBusRx.isSynchro()) /* One Serial frame just arrived */
  {
    SerialFrameCnt++;
    if(SerialFrameCnt >= DISPLAY_EVERY_SERIAL_FRAME_NB)
    {
      SerialFrameCnt = 0;
      /* Display SBUS channels and flags in the serial console */
      for(uint8_t Ch = 1; Ch <= RcBusRx.channelNb(); Ch++)
      {
        Serial.print(F("Ch["));Serial.print(Ch);Serial.print(F("]="));Serial.print(RcBusRx.width_us(Ch));Serial.print(F(" Raw="));Serial.println(RcBusRx.rawData(Ch));
      }
      Serial.print(F("Ch17="));    Serial.println(RcBusRx.flags(SBUS_RX_CH17)); /* Digital Channel#17 */
      Serial.print(F("Ch18="));    Serial.println(RcBusRx.flags(SBUS_RX_CH18)); /* Digital Channel#18 */
      Serial.print(F("FrmLost=")); Serial.println(RcBusRx.flags(SBUS_RX_FRAME_LOST)); /* Switch off the Transmitter to check this */
      Serial.print(F("FailSafe="));Serial.println(RcBusRx.flags(SBUS_RX_FAILSAFE));   /* Switch off the Transmitter to check this */
      Serial.println();
    }
  }
}
