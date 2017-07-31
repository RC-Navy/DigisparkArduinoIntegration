#include <Rcul.h>
#include <SoftRcPulseIn.h>
#ifndef ESP8266
#include <TinyPinChange.h>
#endif

#define BROCHE_VOIE1  2

SoftRcPulseIn ImpulsionVoie1;


void setup()
{
#if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__)
    Serial.begin(9600);
    Serial.print(F("SoftRcPulseIn lib V"));Serial.print(SOFT_RC_PULSE_IN_VERSION);Serial.print(F("."));Serial.print(SOFT_RC_PULSE_IN_REVISION);Serial.println(F(" demo"));
#endif
  ImpulsionVoie1.attache(BROCHE_VOIE1);
}

void loop()
{
  if(ImpulsionVoie1.disponible())
  {
#if !defined(__AVR_ATtiny24__) && !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny84__) && !defined(__AVR_ATtiny25__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny85__)
    Serial.print("Pulse=");Serial.println(ImpulsionVoie1.largeur_us());
#endif
  }
}


