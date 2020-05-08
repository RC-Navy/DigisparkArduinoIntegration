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
There is no need to change the SBUS servo channel since all the 16 SBUS channels will be set with value defined
by the potentiometer position.

Francais:
========
Ce sketch est la version SBUS de l'exemple bien connu du sketch "Knob" fourni avec la bibliotheque "Servo".
Le but est de piloter un servo SBUS avec un potentiometre.
Le signal SBUS est disponible sur la broche Tx de l'UART Serial (habituellement la broche#1 sur la plupart des arduinos).
Il n'y a pas besoin de changer la voie SBUS du servo puisque les 16 voies du SBUS vont etre chargees par la valeur definie
par la position du potentiometre.

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
    for(uint8_t Ch = 1; Ch <= SBUS_TX_CH_NB; Ch++)
    {
      SBusTx.width_us(Ch, Val); /* Set the value to all the 16 SBUS channels */
    }
    SBusTx.sendChannels();               /* Send the SBUS frame  */
  }
}

