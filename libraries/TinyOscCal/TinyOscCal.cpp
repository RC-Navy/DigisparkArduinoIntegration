#include "TinyOscCal.h"

/*
 English: by RC Navy (2012)
 =======
 <TinyOscCal>: a library to easily calibrate the internal Oscillator of the ATtiny.
 http://p.loussouarn.free.fr

 Francais: par RC Navy (2012)
 ========
 <TinyOscCal>: une bibliotheque pour calibrer facilement l'oscillateur interne des ATtiny.
 http://p.loussouarn.free.fr
*/

enum {TINY_OSC_CAL_EEP_INIT_OSCCAL_OFFSET = 0, TINY_OSC_CAL_EEP_NB}; /* EEPROM storage indexes */

#define OSCCAL_OFFSET_RANGE	 20   /* OSCCAL offset will go from -OSCCAL_OFFSET_RANGE to +OSCCAL_OFFSET_RANGE */
#define STORAGE_OFFSET		 128  /* Used to detect cleared EEPROM: 0xFF = -1 which is within [-OSCCAL_OFFSET_RANGE, +OSCCAL_OFFSET_RANGE] */

/*************************************************************************
				GLOBAL VARIABLES
*************************************************************************/
TinyOscCalClass TinyOscCal = TinyOscCalClass();
 
/* Constructor */
TinyOscCalClass::TinyOscCalClass()
{
 
}

uint8_t TinyOscCalClass::init(SoftSerial *MySoftSerial, uint8_t EepromStartIdx, int8_t ForceClear /* =0 */)
{
uint8_t Ret = TINY_OSC_CAL_INIT_CALIBRATION_NOT_DONE;
#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
int8_t  FactoryOscCal, OscCalOffset;
long    StartMs;
int8_t  Done = 0;

	/* Internal Oscillator calibration */
	FactoryOscCal = OSCCAL;
	if(ForceClear) EEPROM.write(EepromStartIdx + TINY_OSC_CAL_EEP_INIT_OSCCAL_OFFSET, 0xFF);
	OscCalOffset = (int8_t)EEPROM.read(EepromStartIdx + TINY_OSC_CAL_EEP_INIT_OSCCAL_OFFSET) - STORAGE_OFFSET; /* Cleared EEPROM gives: 255 - 128 = 127  => > OSCCAL_OFFSET_RANGE */
	if( (OscCalOffset < (-OSCCAL_OFFSET_RANGE)) || (OscCalOffset > OSCCAL_OFFSET_RANGE) )
	{
		for(OscCalOffset = -OSCCAL_OFFSET_RANGE; OscCalOffset <= OSCCAL_OFFSET_RANGE; OscCalOffset++)
		{
			OSCCAL = FactoryOscCal + OscCalOffset; /* Apply Offset to internal oscillator */
			delay(1); /* Wait for internal oscillator stabilization */
			MySoftSerial->txMode();
			MySoftSerial->print(F("Offset="));MySoftSerial->print((int)OscCalOffset);MySoftSerial->println(F(" hit <Enter>"));
			MySoftSerial->rxMode();
			delay(1000);
			if(MySoftSerial->available() > 0)
			{
				EEPROM.write(EepromStartIdx + TINY_OSC_CAL_EEP_INIT_OSCCAL_OFFSET, OscCalOffset + STORAGE_OFFSET); /* Memo the OSCCAL Offset */
				Done = 1;
				Ret = TINY_OSC_CAL_INIT_CALIBRATION_DONE;
				MySoftSerial->txMode();
				MySoftSerial->println(F("CAL: OK"));
			}
			if(Done) break; /* Exit ASAP */
		}
	}
	else
	{
		/* Apply OSCCAL offset from EEPROM */
		OSCCAL = FactoryOscCal + OscCalOffset;
		Ret = TINY_OSC_CAL_INIT_CURRENT_FROM_EEPROM;
	}
#endif
   return(Ret);
}

uint8_t TinyOscCalClass::getEepromStorageSize()
{
  return(TINY_OSC_CAL_EEP_NB);
}
