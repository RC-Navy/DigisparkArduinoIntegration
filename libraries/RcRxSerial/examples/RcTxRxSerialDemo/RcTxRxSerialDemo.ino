#include <Rcul.h>

#include <TinyPpmGen.h>
#include <RcTxSerial.h>

#include <TinyPinChange.h>
#include <TinyPpmReader.h>
#include <RcRxSerial.h>

// by RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html)
// This sketch can work with an Arduino UNO (can be adapted for other arduino).
// This example code is in the public domain.

/*
GB:
==
This sketch demonstrates how to send a text message using a channel of a PPM frame at transmitter side and how to receive it at receiver side.
This sketch generates a PPM frame transporting 5 channels and uses the 5th channel to transport text messages.
TinyPpmGen and RcTxserial are used to generate the PPM frame (PPM sum) on pin6 of an arduino UNO.
On the arduino UNO, the pin6 is connected to the pin2.
TinyPpmReader and RcRxSerial are the receiver part and extracts channels 1 to 4 and the message from channel 5.

FR:
==
Ce sketch démontre comment envoyer un message texte via un canal d'un train PPM cté émetteur et comment le recevoir cté récepteur.
Ce sketch génère un train PPM transportant 5 canaux et utilise le 5e canal pour transporter le message texte.
TinyPpmGen et RcTxserial sont utilisés pour générer le train PPM (PPM sum) sur la broche6 d'un arduino UNO.
Sur l'arduino UNO, la broche6 est connectée à la broche2.
TinyPpmReader et RcRxSerial sont la partie réception et extraient les canaux 1 à 4 et le message texte depuis le canal 5.

                           Transmitter simulation (PPM Generation)      Extraction of Channels 1 to 4 and msg from Channel 5         
                            .----------------^-----------------.               .----------------^-----------------.
                                                                                         <TinyPinChange.h>
                                       <TinyPpmGen.h>                                    <TinyPpmReader.h>
                                          <Rcul.h>                                          <Rcul.h>
                                      .---------------.                                 .---------------.
                                      |               |      Pin6 connected to Pin2     |               |
  Values of Channels 1 to 4 --------->|  TinyPpmGen   |----> PPM Frame (5 channels) --->| TinyPpmReader |---------> Values of Channels 1 to 4
                                      |               |       (PPM sum signal)          |               |
                                      '---------------'                                 '---------------'
                                              ^                                                 | Use the 5th channel to receive messsages
                                              | Use the 5th channel to send messsages           V
                                      .---------------.                                 .---------------.
                                      |               |                                 |               |
        MyRcTxSerial.print("msg") --->| MyRcTxSerial  |                                 | MyRcRxSerial  |---> MyRcRxSerial.msgAvailable()
                                      |               |                                 |               |
                                      '---------------'                                 '---------------'
                                       <RcTxSerial.h>                                    <RcRxSerial.h>
Note:
====

GB:
==
In the true life,
- The left part is located in a RC Transmitter and acts as a PPM modulator,
- The right part is connected to the PPM Sum output of an RC Receiver.
But, for the demonstration, both parts are grouped in the same sketch in order to use a single arduino.
Please, note the RcRxSerial can be "attached" to a regular receiver channel output by using the SoftRcPulseIn library as described below:

FR:
==
Dans la pratique,
- La partie gauche est située dans l'émetteur RC et agit comme un modulateur PPM,
- la partie droite est connectée à la sortie PPM Sum d'un récepteur RC.
Mais, pour la démonstration, les 2 paries ont été groupées dans la meme sketch afin de n'utiliser qu'un seul arduino.
Notez que le RcRxSerial peut etre "attaché" à une sortie voie d'un récepteur an utilisant la bibliothèque SoftRcPulseIn comme décrit ci-dessous:

SoftRcPulseIn MyRcChannel;
RcRxSerial    MyRcRxSerial(&MyRcChannel);

void setup()
{
  MyRcChannel.attach(3); // if pin 3 connected to the receiver
  ...
}
The utilization of the MyRcRxSerial object (the RX Part below) remains exactly the same!
*/
#define PPM_INPUT_PIN   2

#define DATA_RC_CHANNEL 5

RcTxSerial MyRcTxSerial(&TinyPpmGen, 16, DATA_RC_CHANNEL); /* Create a Tx serial port with a tx fifo of 16 bytes on the channel#5 of the TinyPpmGen  (/!\ Data rate = 200 bauds /!\) */

RcRxSerial MyRcRxSerial(&TinyPpmReader,  DATA_RC_CHANNEL); /* Create a Rx serial port on the channel#5 of the TinyPpmReader */


#define PULSE_WIDTH_MIN_US    1000
#define PULSE_WIDTH_MAX_US    2000
#define STEP_US               5

uint16_t Width_us = PULSE_WIDTH_MAX_US;
uint16_t Step_us  = STEP_US;

#define RX_MSG_LEN_MAX 10
char    RxMsg[RX_MSG_LEN_MAX];
uint8_t RxLen;

uint32_t StartMs = millis();

void setup()
{
  TinyPpmGen.begin(TINY_PPM_GEN_POS_MOD, DATA_RC_CHANNEL); /* Generate 5 channels. Change TINY_PPM_GEN_POS_MOD to TINY_PPM_GEN_NEG_MOD for NEGative PPM modulation */
  TinyPpmReader.attach(PPM_INPUT_PIN); /* Attach MyPpmReader to PPM_INPUT_PIN pin */
  Serial.begin(115200);
}

void loop()
{
  /***********/
  /* TX Part */
  /***********/
  if(TinyPpmGen.isSynchro())
  {
    TinyPpmGen.setChWidth_us(1, 500);
    TinyPpmGen.setChWidth_us(2, Width_us);       /* Channel#2 sweeps between 2000 and 1000 us */
    TinyPpmGen.setChWidth_us(3, 3000 - Width_us);/* Channel#3 sweeps between 1000 and 2000 us */
    TinyPpmGen.setChWidth_us(4, 2000);
    Width_us += Step_us;
    if(Width_us > PULSE_WIDTH_MAX_US) Step_us = -STEP_US;
    if(Width_us < PULSE_WIDTH_MIN_US) Step_us = +STEP_US;
  }
  if(millis() - StartMs >= 250) /* /!\ 250ms Clock used to not flood the serial link (Data rate # 200 Bauds) /!\ */
  {
    StartMs = millis(); /* Restart chrono */
    /* RC Channel#5: send a message (milliseconds elapsed since the power-up) using PCM over PPM (TM: RC Navy) */
    MyRcTxSerial.print(millis()%100000); /* Modulo 100000 to limit message length to 5 characters (0 to 99999) */
  }
  RcTxSerial::process();
  
  /***********/
  /* RX Part */
  /***********/
  if(RxLen = MyRcRxSerial.msgAvailable(RxMsg, RX_MSG_LEN_MAX))
  {
    /* Display Channel 1 to 4 */
    for(uint8_t Ch = 1; Ch < DATA_RC_CHANNEL; Ch++)
    {
      Serial.print(F("Ch"));Serial.print(Ch);Serial.print(F("="));Serial.println(TinyPpmReader.width_us(Ch));
    }
    /* Display message received via the 5th channel */
    RxMsg[RxLen] = 0; /* Set End of String before display in the serial console */
    Serial.print(F("Rx Msg="));Serial.println(RxMsg);
  }
}
