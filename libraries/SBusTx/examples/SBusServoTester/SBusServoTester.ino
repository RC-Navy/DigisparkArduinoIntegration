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
This sketch is the SBUS version of the wellknown "Knob" example sketch provided with the "Servo" library.
The aim is to drive a SBUS servo with a potentiometer.
The SBUS signal is available on the Tx pin of the Serial UART (usually the pin#1 on most of the arduinos).
The SBUS servo shall be configured to use the channel defined with the SBUS_SERVO_CH macro.
(or the macro SBUS_SERVO_CH shall be modified to match with the channel configured in the SBUS servo)

Francais:
========
Ce sketch est la version SBUS de l'exemple bien connu du sketch "Knob" fourni avec la bibliotheque "Servo".
Le but est de piloter un servo SBUS avec un potentiometre.
Le signal SBUS est disponible sur la broche Tx de l'UART Serial (habituellement la broche#1 sur la plupart des arduinos).
Le servo SBUS doit etre configure pour utiliser la voie definie par la macro SBUS_SERVO_CH.
(ou la macro SBUS_SERVO_CH doit etre modifiee pour correspondre a la voie configuree dans le servo SBUS)

Wiring/Cablage:
==============                                                  _______
                                                               '_______'   
     .------------------------.                      .------------' '---.  
     |         ARDUINO        |                      |                  |
     |                        |                      |     SBUS Servo   |
     |       Serial Tx(Pin#1) +----------------------+ SBUS             |
     |                        |                      |                  |
     |                    +5V +---------.        .---+ +6V              |
     |                        |         |        |   |                  |
     |                    GND +---o----------o-------+ GND              |
     |                        |   |     |    |   |   '------------------'
     |                        |   |     |    |   |        .---------------------------.
     |                     A0 +---|--.  |    |   |        |                           |
     '------------------------'  _|__|__|_   |   '--------+ +6V to +7.4V              |
                                '         '  |            |      External Battery     |
                                '_________'  '------------+ GND                       |
                                   |   |                  |                           |
                                   '---'                  '---------------------------'
                                    | | Potentiometer
                                    | |
                                    '-'
*/
#include <SBusTx.h>


#define SBUS_SERVO_CH    5 /* The SBUS servo shall be configured for this channel, or adjust here SBUS_SERVO_CH for this */

int PotPin = A0; /* analog pin used to connect the potentiometer */
int Val;         /* variable to read the value from the analog pin */

void setup()
{
  Serial.begin(100000, SERIAL_8E2); /* Initialization with the right SBUS settings of the Serial UART used for the SBUS generator */
  SBusTx.serialAttach(&Serial, SBUS_TX_NORMAL_TRAME_RATE_MS); /* Attach the SBUS generator to the Serial with SBUS_TX_NORMAL_TRAME_RATE_MS or SBUS_TX_HIGH_SPEED_TRAME_RATE_MS */
}

void loop()
{ 
  if(SBusTx.isSynchro()) /* The previous SBUS frame is sent */
  {
    Val = analogRead(PotPin);            /* Reads the value of the potentiometer (value between 0 and 1023) */
    Val = map(Val, 0, 1023, 1000, 2000); /* Scale it to use it with the servo (value between 1000 and 2000) */
    SBusTx.width_us(SBUS_SERVO_CH, Val); /* Set the SBUS channel */
    SBusTx.sendChannels();               /* Send the SBUS frame  */
  }
}

