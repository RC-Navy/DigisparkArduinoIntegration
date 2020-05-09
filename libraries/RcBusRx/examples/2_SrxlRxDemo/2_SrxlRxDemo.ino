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
This sketch demonstrates how to use the <RcBusRx> library when it is configured for SRXL.
This sketch requires 2 Serial ports on the Arduino, so it works on Arduino Leonardo and Arduino MEGA.
The aim is to display on the Serial Console, the values of the n RC Channels.
Just connect the Receiver SRXL signal to the Rx pin of the Serial1 UART on the arduino.

Francais:
========
Ce sketch demontre comment utiliser la bibliotheque <RcBusRx> quand elle est configuree en SRXL.
Ce sketch necessite 2 ports serie sur l'Arduino, il fonctionne donc sur arduino Leonardo et Arduino MEGA.
Le but est d'afficher dans la console serie, les valeurs des n voies RC.
Connecter simplement le signal SRXL du recepteur a la broche Rx de l'UART Serial1 de l'arduino.

Wiring/Cablage:
==============
 .---------------.              .----------------------.           .------------------.  
 |  RC Receiver  |              |    Leonardo/MEGA     |           |       PC         |
 |               |              |                      |           |                  | 
 |    SRXL Signal+------------->+ Serial1 Rx           |           | (Serial Console) |
 |               |              |                   USB+-----------+USB               |
 |            Gnd+--------------+ Gnd                  | USB cable |                  |
 |               |              |                      |           |                  |
 '---------------'              '----------------------'           '------------------'

*/
#include <RcBusRx.h>

void setup()
{
  while(!Serial);
  Serial.begin(115200); /* For serial console output */
  Serial.println(F("RcBusRx configured in SRXL receiver\n"));
  
  Serial1.begin(SRXL_RX_SERIAL_CFG); /* Choose your serial and select xxxx__RX_SERIAL_CFG (list in RcBusRx.h) where xxxx is the protocol */
  
  RcBusRx.serialAttach(&Serial1);    /* Then, attach the SRXL receiver to this Serial1 */
  RcBusRx.setProto(RC_BUS_RX_SRXL);  /* Select here the serial protocol */
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
      /* Display SRXL channels in the serial console */
      for(uint8_t Ch = 1; Ch <= RcBusRx.channelNb(); Ch++)
      {
        Serial.print(F("Ch["));Serial.print(Ch);Serial.print(F("]="));Serial.print(RcBusRx.width_us(Ch));Serial.print(F(" Raw="));Serial.println(RcBusRx.rawData(Ch));
      }
      Serial.println();
    }
  }
}
