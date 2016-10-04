/*
 English: by RC Navy (2012-2015)
 =======
 <SoftRcPulseIn>: an asynchronous library to read Input Pulse Width from standard Hobby Radio-Control. This library is a non-blocking version of pulseIn().
 http://p.loussouarn.free.fr
 V1.2: (06/04/2015) Support Rcul ajoute (permet de creer un port serie virtuel par dessus une voie PPM)

 Francais: par RC Navy (2012-2015)
 ========
 <SoftRcPulseIn>: une librairie asynchrone pour lire les largeur d'impulsions des Radio-Commandes standards. Cette librairie est une version non bloquante de pulsIn().
 http://p.loussouarn.free.fr
 06/04/2015: Rcul support added (allows to create a virtual serial port over a PPM channel)
 V1.3: (12/04/2016) boolean type replaced by uint8_t and version management replaced by constants
*/

#ifndef SOFT_RC_PULSE_IN_H
#define SOFT_RC_PULSE_IN_H

#include "Arduino.h"
#include <TinyPinChange.h>
#include <Rcul.h>

#include <inttypes.h>

#define SOFT_RC_PULSE_IN_TIMEOUT_SUPPORT

#define SOFT_RC_PULSE_IN_VERSION                    1
#define SOFT_RC_PULSE_IN_REVISION                   4

class SoftRcPulseIn : public Rcul
{
  public:
    SoftRcPulseIn();
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
    class SoftRcPulseIn  *next;
    static SoftRcPulseIn *first;
    uint8_t _Pin;
    uint8_t _PinMask;
    int8_t  _VirtualPortIdx;
    uint16_t _Min_us;
    uint16_t _Max_us;
    uint32_t _Start_us;
    uint32_t _Width_us;
    uint8_t  _Available;
#ifdef SOFT_RC_PULSE_IN_TIMEOUT_SUPPORT
    uint8_t _LastTimeStampMs;
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
