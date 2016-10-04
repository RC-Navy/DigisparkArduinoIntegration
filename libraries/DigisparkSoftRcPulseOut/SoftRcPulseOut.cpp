#include <SoftRcPulseOut.h>
/*
 Update 01/03/2013: add support for Digispark (http://digistump.com): automatic Timer selection (RC Navy: p.loussouarn.free.fr)
 Update 19/08/2014: usage with write_us and read_us fixed
 Update 06/04/2015: Rcul support added (allows to create a virtual serial port over a PPM channel)
 Update 03/06/2015: add support for dynamic object creation/destruction
                   (createInstance, destroyInstance, createdInstanceNbmethods, softRcPulseOutById and getIdByPin methods added)
 
 English: by RC Navy (2012) 
 =======
 <SoftRcPulseOut>: a library mainly based on the <SoftwareServo> library, but with a better pulse generation to limit jitter.
 It supports the same methods as <SoftwareServo>.
 It also support Pulse Width order given in microseconds. The current Pulse Width can also be read in microseconds.
 The refresh method can admit an optionnal argument (force). If SoftRcPulseOut::refresh(1) is called, the refresh is forced even if 20 ms are not elapsed.
 The refresh() method returns 1 if refresh done (can be used for synchro and/or for 20ms timer).
 http://p.loussouarn.free.fr

 Francais: par RC Navy (2012)
 ========
 <SoftRcPulseOut>: une librairie majoritairement basee sur la librairie <SoftwareServo>, mais avec une meilleure generation des impulsions pour limiter la gigue.
 Elle supporte les memes methodes que <SoftwareServo>.
 Elle supporte egalement une consigne de largeur d'impulsion passee en microseconde. La largeur de l'impulsion courante peut egalement etre lue en microseconde.
 La methode refresh peut admettre un parametre optionnel (force). Si SoftRcPulseOut::resfresh(1) est appelee, le refresh est force meme si 20 ms ne se sont pas ecoulee.
 La methode refresh() retourne 1 si refresh effectue (peut etre utilise pour synhro et/ou 20ms timer).
 http://p.loussouarn.free.fr
*/

/* Automatic Timer selection (at compilation time) */
#ifndef TIMER_TO_USE_FOR_MILLIS //This symbol is not defined arduino standard core and is defined in core_build_options.h in DigiStump version
#define SOFT_RC_PULSE_OUT_TCNT TCNT0 //For arduino standard core of UNO/MEGA, etc
#else
#if (TIMER_TO_USE_FOR_MILLIS==1)
#define SOFT_RC_PULSE_OUT_TCNT TCNT1 //For example for ATtiny85
#else
#define SOFT_RC_PULSE_OUT_TCNT TCNT0 //For example for ATtiny84 and ATtiny167
#endif
#endif

SoftRcPulseOut *SoftRcPulseOut::first;

typedef struct{
  uint8_t
    Total:       4,
    Dynamically: 4;
}ObjectCreatedSt_t;

static ObjectCreatedSt_t ObjectCreated = {0, 0};

#define NO_ANGLE                           (0xff)
#define NOT_ATTACHED                       (0xff)

SoftRcPulseOut::SoftRcPulseOut()
{
  pin    = NOT_ATTACHED;
  pulse0 = 0;
  next   = first;
  first  = this;
  ObjectCreated.Total++;
}

void SoftRcPulseOut::setMinimumPulse(uint16_t t)
{
    min16 = t / 16;
}

void SoftRcPulseOut::setMaximumPulse(uint16_t t)
{
    max16 = t / 16;
}

int8_t SoftRcPulseOut::createInstance(void)
{
  int8_t Ret = -1; /* In case of failure */
  SoftRcPulseOut *p;
  
  if(ObjectCreated.Total < SOFT_RC_PULSE_OUT_INSTANCE_MAX_NB)
  {
    p = new SoftRcPulseOut; /* new calls the constructor which increments ObjectCreated */
    if(p)
    {
      ObjectCreated.Dynamically++;
      Ret = ObjectCreated.Total - 1;
    }
  }
  return(Ret);
}

uint8_t SoftRcPulseOut::createdInstanceNb(void)
{
  return(ObjectCreated.Total);
}

uint8_t SoftRcPulseOut::destroyInstance(uint8_t ObjIdx)
{
  int8_t Idx, Ret = 0;
  SoftRcPulseOut *This;

  if((ObjIdx >= (ObjectCreated.Total - ObjectCreated.Dynamically)) && (ObjIdx < ObjectCreated.Total))
  {
    This = softRcPulseOutById(ObjIdx);
    if(!This->attached())
    {
      for ( SoftRcPulseOut **p = &first; *p != 0; p = &((*p)->next) )
      {
	if ( *p == This )
	{
	  *p = This->next;
	  This->next = 0;
	  delete(This);
	  ObjectCreated.Dynamically--;
	  ObjectCreated.Total--;
	  Ret = 1;
	  break;
	}
      }
    }
  }
  return(Ret);
}

SoftRcPulseOut *SoftRcPulseOut::softRcPulseOutById(uint8_t ObjIdx)
{
  int8_t Idx;
  SoftRcPulseOut *p;
  
  if(ObjIdx < ObjectCreated.Total)
  {
    Idx = ObjectCreated.Total - ObjIdx -1;
    if(Idx >= 0)
    {
      p = first;
      for(uint8_t i = 0; i < Idx ;i++)
      {
	p = p->next;
      }
      return(p);
    }
  }
  return(NULL);  
}  

int8_t SoftRcPulseOut::getIdByPin(uint8_t Pin)
{
  int8_t Idx =  0, Id = -1;
  SoftRcPulseOut *p;
  
  for ( p = first; p != 0; p = p->next )
  {
    Idx++;
    if( p->pin == Pin)
    {
      Id = ObjectCreated.Total - Idx;
      break;
    }
  }
  return(Id);
}

uint8_t SoftRcPulseOut::attach(int pinArg)
{
    pin    = pinArg;
    angle  = NO_ANGLE;
    min16  = 34;
    max16  = 150;
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
    return (1);
}

void SoftRcPulseOut::detach()
{
  pin    = NOT_ATTACHED;
}

void SoftRcPulseOut::write(int angleArg)
{
    if (angleArg < 0)   angleArg = 0;
    if (angleArg > 180) angleArg = 180;
    angle = angleArg;
    // bleh, have to use longs to prevent overflow, could be tricky if always a 16MHz clock, but not true
    // That 64L on the end is the TCNT0 prescaler, it will need to change if the clock's prescaler changes,
    // but then there will likely be an overflow problem, so it will have to be handled by a human.
#ifdef MS_TIMER_TICK_EVERY_X_CYCLES
    pulse0 = (min16 * 16L * clockCyclesPerMicrosecond() + (max16 - min16) * (16L * clockCyclesPerMicrosecond()) * angle / 180L) / MS_TIMER_TICK_EVERY_X_CYCLES;
#else
    pulse0 = (min16 * 16L * clockCyclesPerMicrosecond() + (max16 - min16) * (16L * clockCyclesPerMicrosecond()) * angle / 180L) / 64L;
#endif
}

void SoftRcPulseOut::write_us(uint16_t PulseWidth_us)
{
    if ( PulseWidth_us < (min16 * 16)) PulseWidth_us = (min16 * 16);
    if ( PulseWidth_us > (max16 * 16)) PulseWidth_us = (max16 * 16);
#ifdef MS_TIMER_TICK_EVERY_X_CYCLES
    pulse0 = (PulseWidth_us * clockCyclesPerMicrosecond()) / MS_TIMER_TICK_EVERY_X_CYCLES;
#else
    pulse0 = (PulseWidth_us * clockCyclesPerMicrosecond()) / 64L;
#endif
    angle = map(PulseWidth_us, min16 * 16, max16 * 16, 0, 180);
}

uint8_t SoftRcPulseOut::read()
{
    return angle;
}

uint16_t SoftRcPulseOut::read_us()
{
#ifdef MS_TIMER_TICK_EVERY_X_CYCLES
    return((pulse0 * MS_TIMER_TICK_EVERY_X_CYCLES) / clockCyclesPerMicrosecond());
#else
    return((pulse0 * 64L) / clockCyclesPerMicrosecond());
#endif
}

uint8_t SoftRcPulseOut::attached()
{
  return(pin != NOT_ATTACHED);
}

/* Begin of Rcul support */
uint8_t SoftRcPulseOut::RculIsSynchro()
{
  return(refresh());
}

void SoftRcPulseOut::RculSetWidth_us(uint16_t Width_us, uint8_t Ch /*= 255*/)
{
  Ch = Ch; /* To avoid a warning at compilation time */
  write_us(Width_us);
}

uint16_t SoftRcPulseOut::RculGetWidth_us(uint8_t Ch)
{
  Ch = Ch; /* To avoid a compilation warning */
  return(0);
}
/* End of Rcul support */

uint8_t SoftRcPulseOut::refresh(bool force /* = false */)
{
  uint8_t  RefreshDone = 0;
  uint8_t  count = 0, i = 0;
  uint16_t base = 0;
  SoftRcPulseOut *p;
  static uint32_t lastRefresh = 0;
  uint32_t m = millis();
  
  if(!force)
  {
    // if we haven't wrapped millis, and 20ms have not passed, then don't do anything
    if ( (m - lastRefresh) < 20UL ) return(RefreshDone);
  }
  RefreshDone = 1; //Ok: Refresh will be performed
  lastRefresh = m;

  for ( p = first; p != 0; p = p->next ) if ( p->pin != NOT_ATTACHED ) count++;
  if ( count == 0 ) return(RefreshDone);

  // gather all the SoftRcPulseOuts in an array
  SoftRcPulseOut *s[count];
  for ( p = first; p != 0; p = p->next ) if ( p->pin != NOT_ATTACHED ) s[i++] = p;

  // bubblesort the SoftRcPulseOuts by pulse time, ascending order
  s[0]->ItMasked = 0;
  for(;;)
  {
    uint8_t moved = 0;
    for ( i = 1; i < count; i++ )
    {
      s[i]->ItMasked = 0;
      if ( s[i]->pulse0 < s[i - 1]->pulse0 )
      {
	SoftRcPulseOut *t = s[i];
	s[i] = s[i - 1];
	s[i - 1] = t;
	moved = 1;
      }
    }
    if ( !moved ) break;
  }
  for ( i = 1; i < count; i++ )
  {
    if ( abs(s[i]->pulse0 - s[i - 1]->pulse0) <= 5)
    {
      s[i]->ItMasked = 1; /* 2 consecutive Pulses are close each other, so do not unmask interrupts between Pulses */
    }
  }
  // turn on all the pins
  // Note the timing error here... when you have many SoftwareServos going, the
  // ones at the front will get a pulse that is a few microseconds too long.
  // Figure about 4uS/SoftRcPulseOut after them. This could be compensated, but I feel
  // it is within the margin of error of software SoftRcPulseOuts that could catch
  // an extra interrupt handler at any time.
  noInterrupts();
  for ( i = 0; i < count; i++ ) digitalWrite( s[i]->pin, 1);
  interrupts();

  uint8_t start = SOFT_RC_PULSE_OUT_TCNT;
  uint8_t now = start;
  uint8_t last = now;

  // Now wait for each pin's time in turn..
  for ( i = 0; i < count; i++ )
  {
    uint16_t go = start + s[i]->pulse0;
#ifndef MS_TIMER_TICK_EVERY_X_CYCLES
    uint16_t it = go - 4; /* 4 Ticks is OK for UNO @ 16MHz with default prescaler*/ /* Mask Interruptions just before setting down the pin */
#else
    uint16_t it = go - max(4, (256 / MS_TIMER_TICK_EVERY_X_CYCLES)); /* 4 Ticks is OK for UNO @ 16MHz */ /* Mask Interruptions just before setting down the pin */
#endif
    // loop until we reach or pass 'go' time: this is a blocking loop (max 2400us) except for non masked ISR (between edges)
    for (;;)
    {
      now = SOFT_RC_PULSE_OUT_TCNT;
      if ( now < last ) base += 256;
      last = now;
      if( !s[i]->ItMasked )
      {
	if( base + now > it)
	{
	  noInterrupts();
	  s[i]->ItMasked = 1;
	}
      }
      if ( base + now > go )
      {
	digitalWrite( s[i]->pin, 0);
	if( (i + 1) < count )
	{
	  if( !s[i + 1]->ItMasked )
	  {
	    interrupts();
	  }
	}else interrupts();
	break;
      }
    }
  }
  return(RefreshDone);
}
