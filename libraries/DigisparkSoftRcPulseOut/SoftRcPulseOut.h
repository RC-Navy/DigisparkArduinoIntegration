#ifndef SoftRcPulseOut_h
#define SoftRcPulseOut_h

/*
 Update 01/03/2013: add support for Digispark (http://digistump.com): automatic Timer selection (RC Navy: p.loussouarn.free.fr)
 Update 19/08/2014: usage with write_us and read_us fixed and optimized for highest resolution
 Update 06/04/2015: RcTxPop support added (allows to create a virtual serial port over a PPM channel)
 Update 03/06/2015: add support for dynamic object creation/destruction
                    (createInstance, destroyInstance, createdInstanceNbmethods, softRcPulseOutById and getIdByPin methods added)

 English: by RC Navy (2012/2015)
 =======
 <SoftRcPulseOut>: a library mainly based on the <SoftwareServo> library, but with a better pulse generation to limit jitter.
 It supports the same methods as <SoftwareServo>.
 It also support Pulse Width order given in microseconds. The current Pulse Width can also be read in microseconds.
 The refresh method can admit an optionnal argument (force). If SoftRcPulseOut::refresh(1) is called, the refresh is forced even if 20 ms are not elapsed.
 The refresh() method returns 1 if refresh done (can be used for synchro and/or for 20ms timer).
 http://p.loussouarn.free.fr

 Francais: par RC Navy (2012/2015)
 ========
 <SoftRcPulseOut>: une librairie majoritairement basee sur la librairie <SoftwareServo>, mais avec une meilleure generation des impulsions pour limiter la gigue.
 Elle supporte les memes methodes que <SoftwareServo>.
 Elle supporte egalement une consigne de largeur d'impulsion passee en microseconde. La largeur de l'impulsion courante peut egalement etre lue en microseconde.
 La methode refresh peut admettre un parametre optionnel (force). Si SoftRcPulseOut::resfresh(1) est appelee, le refresh est force meme si 20 ms ne se sont pas ecoulee.
 La methode refresh() retourne 1 si refresh effectue (peut etre utilise pour synhro et/ou 20ms timer).
 http://p.loussouarn.free.fr
*/

#include "Arduino.h"
#include <Rcul.h>

#include <inttypes.h>

#define SOFT_RC_PULSE_OUT_VERSION          2
#define SOFT_RC_PULSE_OUT_REVISION         0

#define SOFT_RC_PULSE_OUT_INSTANCE_MAX_NB  15 /* Counter on 4 bits */

class SoftRcPulseOut : public Rcul
{
  private:
    boolean               ItMasked;
    uint8_t               pin;
    uint8_t               angle;      // in degrees
    uint16_t              pulse0;     // pulse width in TCNT0 counts
    uint8_t               min16;      // minimum pulse, 16uS units  (default is 34)
    uint8_t               max16;      // maximum pulse, 16uS units, 0-4ms range (default is 150)
    class                 SoftRcPulseOut *next;
    static                SoftRcPulseOut *first;
  public:
    SoftRcPulseOut();
    uint8_t               attach(int);        // attach to a pin, sets pinMode, returns 0 on failure, won't
                                              // position the servo until a subsequent write() happens
    void                  detach();
    void                  write(int);         // specify the angle in degrees, 0 to 180
    void                  write_us(uint16_t); // specify the angle in microseconds, 500 to 2500
    uint8_t               read();             // return the current angle
    uint16_t              read_us();          // return the current pulse with in microseconds
    uint8_t               attached();
    void                  setMinimumPulse(uint16_t);  // pulse length for 0 degrees in microseconds, 540uS default
    void                  setMaximumPulse(uint16_t);  // pulse length for 180 degrees in microseconds, 2400uS default
    /* Rcul support */
    virtual uint8_t       RculIsSynchro();
    virtual void          RculSetWidth_us(uint16_t Width_us, uint8_t Ch = 255);
    virtual uint16_t      RculGetWidth_us(uint8_t Ch);
    static int8_t         createInstance(void);   //Allocate dynamically an instance of a SoftRcPulseOut object. Returns the object id.
    static uint8_t        createdInstanceNb(void);//Return the created instance(s) SoftRcPulseOut object.
    static SoftRcPulseOut *softRcPulseOutById(uint8_t ObjIdx);
    static int8_t         getIdByPin(uint8_t Pin);
    static uint8_t        destroyInstance(uint8_t ObjIdx); //Deallocate dynamically an instance of a SoftRcPulseOut object. Returns the object id.
    static uint8_t        refresh(bool force = false);// must be called at least every 50ms or so to keep servo alive
                                               // you can call more often, it won't happen more than once every 20ms
};

/*      Methodes en Francais                    English native methods */
#define attache                                       attach
#define detache                                       detach
#define ecrit                                         write
#define ecrit_us                                      write_us
#define lit                                           read
#define lit_us                                        read_us
#define estAttache                                    attached
#define definitImpulsionMinimum                       setMinimumPulse
#define definitImpulsionMaximum	                      setMaximumPulse
#define rafraichit                                    refresh
#define creerInstance                                 createInstance
#define nbInstanceCrees                               createdInstanceNb
#define detruireInstance                              destroyInstance
#define softRcPulseOutParId                           softRcPulseOutById
#define idPourBroche                                  getIdByPin

#endif
