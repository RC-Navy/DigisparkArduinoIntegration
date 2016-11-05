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
This sketch demonstrates how to use the <SBusRx> library.
This sketch requires 2 Serial ports on the Arduino, so it works on Arduino Leonardo and Arduino MEGA.
The aim is to display on the Serial Console, the values of the 16 RC Channels and the SBUS flags.
Just connect the Receiver SBUS signal to the Rx pin of the Serial1 UART on the arduino.

Francais:
========
Ce sketch demontre comment utiliser la bibliotheque <>SBusRx.
Ce sketch necessite 2 ports serie sur l'Arduino, il fonctinne donc sur arduino Leonardo et Arduino MEGA.
Le but est d'afficher dans la console serie, les valeurs des 16 voies RC et les indicateurs SBUS.
Connecter simplement le signal SBus du recepteur a la broche Rx de l'UART Serial1 de l'arduino.

Wiring/Cablage:
==============
 .---------------.              .----------------------.           .------------------.  
 |  RC Receiver  |              |    Leonardo/MEGA     |           |       PC         |
 |               |              |                      |           |                  | 
 |    SBus Signal+------------->+ Serial1 Rx           |           | (Serial Console) |
 |               |              |                   USB+-----------+USB               |
 |       SBus Gnd+--------------+ Gnd                  | USB cable |                  |
 |               |              |                      |           |                  |
 '---------------'              '----------------------'           '------------------'

*/
#include <SBusRx.h>

void setup()
{
  Serial.begin(115200); /* For serial console output */
  
  Serial1.begin(100000, SERIAL_8E2); /* Choose your serial first: SBUS works at 100 000 bauds */
  SBusRx.serialAttach(&Serial1); /* Then, attach the SBus receiver to this Serial1 */
}

void loop()
{
  SBusRx.process(); /* Don't forget to call the SBusRx.process()! */
  
  if(SBusRx.isSynchro()) /* One SBUS frame just arrived */
  {
    /* Display SBUS channels and flags in the serial console */
    for(uint8_t Ch = 1; Ch <= SBUS_RX_CH_NB; Ch++)
    {
      Serial.print(F("Ch["));Serial.print(Ch);Serial.print(F("]="));Serial.print(SBusRx.width_us(Ch));Serial.print(F(" Raw="));Serial.println(SBusRx.rawData(Ch));
    }
    Serial.print(F("Ch17="));    Serial.println(SBusRx.flags(SBUS_RX_CH17)); /* Digital Channel#17 */
    Serial.print(F("Ch18="));    Serial.println(SBusRx.flags(SBUS_RX_CH18)); /* Digital Channel#18 */
    Serial.print(F("FrmLost=")); Serial.println(SBusRx.flags(SBUS_RX_FRAME_LOST)); /* Switch off the Transmitter to check this */
    Serial.print(F("FailSafe="));Serial.println(SBusRx.flags(SBUS_RX_FAILSAFE));   /* Switch off the Transmitter to check this */
  }
}
