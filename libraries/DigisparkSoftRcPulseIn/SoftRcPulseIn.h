/*
 English: by RC Navy (2012-2015)
 =======
 <SoftRcPulseIn>: an asynchronous library to read Input Pulse Width from standard Hobby Radio-Control. This library is a non-blocking version of pulseIn().
 http://p.loussouarn.free.fr
 V1.0: initial release
 V1.1: asynchronous timeout support added (up to 250ms)
 V1.2: (06/04/2015) RcTxPop and RcRxPop support added (allows to create a virtual serial port over a PPM channel)
 V1.3: (12/04/2016) boolean type replaced by uint8_t and version management replaced by constants
 V1.4: (04/10/2016) Update with Rcul in replacement of RcTxPop and RcRxPop
 V1.5: (31/06/2017) Support for Arduino ESP8266 added, support of inverted pulse addded
 
 Francais: par RC Navy (2012-2015)
 ========
 <SoftRcPulseIn>: une librairie asynchrone pour lire les largeur d'impulsions des Radio-Commandes standards. Cette librairie est une version non bloquante de pulseIn().
 http://p.loussouarn.free.fr
 V1.0: release initiale
 V1.1: support de timeout asynchrone ajoutee (jusqu'a 250ms)
 V1.2: (06/04/2015) Support de RcTxPop et RcRxPop ajoute (permet de creer un port serie virtuel par dessus une voie PPM)
 V1.3: (12/04/2016) type boolean remplace par uint8_t et gestion de version remplace par des constantes
 V1.4: (04/10/2016) Mise a jour avec Rcul en remplacement de RcTxPop et RcRxPop
 V1.5: (31/06/2017) Ajout du support de l'Arduino ESP8266, ajout du support des impulsions inversees
*/

#ifndef SOFT_RC_PULSE_IN_H
#define SOFT_RC_PULSE_IN_H

#include "Arduino.h"
#include <inttypes.h>
#include <Rcul.h>

#ifndef ESP8266
#include <TinyPinChange.h>
#endif

#define SOFT_RC_PULSE_IN_TIMEOUT_SUPPORT
#define SOFT_RC_PULSE_IN_INV_SUPPORT

#define SOFT_RC_PULSE_IN_VERSION                    1
#define SOFT_RC_PULSE_IN_REVISION                   5

class SoftRcPulseIn : public Rcul
{
  public:
#ifdef SOFT_RC_PULSE_IN_INV_SUPPORT
    SoftRcPulseIn(uint8_t Inv = 0);
#else
    SoftRcPulseIn();
#endif
    static void  SoftRcPulseInInterrupt(void);
    uint8_t      attach(uint8_t Pin, uint16_t PulseMin_us = 600, uint16_t PulseMax_us = 2400);
    uint8_t      available();
    uint8_t      timeout(uint8_t TimeoutMs, uint8_t *State);
    uint16_t     width_us();
    /* Rcul support */
    virtual uint8_t  RculIsSynchro();
    virtual uint16_t RculGetWidth_us(uint8_t Ch);
    virtual void     RculSetWidth_us(uint16_t Width_us, uint8_t Ch = 255);
    private:
    class SoftRcPulseIn  *prev;
    static SoftRcPulseIn *last;
    uint8_t _Pin;
#ifndef ESP8266
    uint8_t _PinMask;
    int8_t  _VirtualPortIdx;
#endif
    uint16_t _Min_us;
    uint16_t _Max_us;
    uint32_t _Start_us;
    uint32_t _Width_us;
    uint8_t  _Available;
#ifdef SOFT_RC_PULSE_IN_TIMEOUT_SUPPORT
    uint8_t _LastTimeStampMs;
#endif
#ifdef SOFT_RC_PULSE_IN_INV_SUPPORT
    uint8_t _Inv;
#endif
};
/*******************************************************/
/* Application Programming Interface (API) en Francais */
/*******************************************************/

/*      Methodes en Francais                            English native methods */
#define attache											attach
#define disponible										available
#define largeur_us										width_us

#endif
