/*
RcPpmReader sketch
by RC Navy (http://p.loussouarn.free.fr/arduino/arduino.html) 2015
This sketch reads an RC PPM frame extracts the numbers of channels and their pulse witdhs.
This sketch can work with a Digispark pro, Digispark, Arduino UNO...
The PPM input shall support pin change interrupt.
This example code is in the public domain.
*/
#include <TinyPinChange.h>
#include <TinyPpmReader.h>
#include <Rcul.h>

#define PPM_INPUT_PIN  2

void setup()
{
  Serial.begin(115200);
  TinyPpmReader.attach(PPM_INPUT_PIN); /* Attach TinyPpmReader to PPM_INPUT_PIN pin */
}

void loop()
{
  TinyPpmReader.suspend(); /* Not needed if an hardware serial is used to display results */
  Serial.print(F("* Period="));Serial.print((int)TinyPpmReader.ppmPeriod_us());Serial.println(F(" us *"));
  Serial.print(F("ChNb="));Serial.println((int)TinyPpmReader.detectedChannelNb());
  for(uint8_t Idx = 1; Idx <= TinyPpmReader.detectedChannelNb(); Idx++) /* From Channel 1 to Max detected */
  {
    Serial.print(F("Ch"));Serial.print(Idx);Serial.print(F("="));Serial.print(TinyPpmReader.width_us(Idx));Serial.println(F(" us"));
  }
  TinyPpmReader.resume(); /* Not needed if an hardware serial is used to display results */
  delay(500);
}
