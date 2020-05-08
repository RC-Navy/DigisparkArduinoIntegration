#ifndef TINY_CPPM_READER
#define TINY_CPPM_READER 1
/* A tiny interrupt driven RC CPPM frame reader library using pin change interrupt
   Features:
   - Uses any input supporting interrupt pin change
   - Supported devices: see TinyPinChange library
   - Positive and negative CPPM modulation supported (don't care)
   - Up to 9 RC channels supported
   RC Navy 2015
   http://p.loussouarn.free.fr
   01/02/2015: Creation
   06/04/2015: Rcul support added (allows to create a virtual serial port over a CPPM channel)
   09/11/2015: No need to create the TinyCppmReader object anymore, unused _PinMask variable removed
   23/03/2020: Multi instance support added (Now, the object(s) shall be created)
*/
#include <Arduino.h>
#include <TinyPinChange.h>
#include <Rcul.h>

#define TINY_CPPM_READER_CH_MAX  9

/* Public function prototypes */
class TinyCppmReader : public Rcul
{
  public:
    TinyCppmReader();
    static uint8_t  attach(uint8_t CppmInputPin);
    static uint8_t  detach(void);
    static uint8_t  detectedChannelNb(void);
    static uint16_t width_us(uint8_t Ch);
    static uint16_t cppmPeriod_us(void);
    static uint8_t  isSynchro(uint8_t ClientIdx = 7); /* Default value: 8th Synchro client -> 0 to 6 free for other clients*/
    static void     suspend(void);
    static void     resume(void);
    static void     rcChannelCollectorIsr(void);
    /* Rcul support */
    virtual uint8_t  RculIsSynchro(uint8_t ClientIdx = RCUL_DEFAULT_CLIENT_IDX);
    virtual uint16_t RculGetWidth_us(uint8_t Ch);
    virtual void     RculSetWidth_us(uint16_t Width_us, uint8_t Ch = RCUL_NO_CH);
  private:
    // static data
};

#endif
