#include <TinyDbg.h>

char MyGlobalString[] = "Global String";

void setup()
{
  Serial.begin(115200);
  TinyDbg_init(&Serial); /* Attach TinyDbg to the wanted debugging Serial port */
  pinMode(13, OUTPUT);
}

void loop()
{
  static uint16_t Count = 0;
  char *myString = (char*)"A standard 'C' string";
  String myString2 = "A String string";
  static uint32_t StartMs = millis(), StartUs = micros(), DurationUs;
  float Float = -PI;
  char Char = ' ';
  TinyDbg_event(); /* May be anywhere in the loop() */
  /* Declare the variables to watch */
  strWATCH(myString);
  u16WATCH(Count);
  rawWATCH(myString2); 
  i32WATCH(DurationUs);
  f32WATCH(Float);
  chrWATCH(Char);

  Count++;
  StartUs = micros();
  BREAK(1); /* First breakpoint */

  Function1();
  Float *= PI; // -PI * -PI: around 10
  DurationUs = micros() - StartUs;
  BREAK(2); /* Second breakpoint */
  if(millis() - StartMs >= 300) /* Heart beat */
  {
    BREAK(3); /* Third breakpoint */
    StartMs = millis();
    digitalWrite(13, !digitalRead(13));
  }
}

void Function1(void)
{
  int16_t LocalVar = 0x1040;
  /* Declare the variables to watch */
  i16WATCH(LocalVar);
  strWATCH(MyGlobalString);
  BREAK(4); /* Fourth breakpoint */
}

