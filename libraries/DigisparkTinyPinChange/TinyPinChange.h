#ifndef TINY_PIN_CHANGE_H
#define TINY_PIN_CHANGE_H 1

/*
* <TinyPinChange>, a library for Pin Change Interrupt by RC Navy (2012)
* Supported devices: ATmega238P (UNO), ATmega2560 (MEGA), ATtiny84, ATtiny85, ATtiny167, ATmega32U4
* 
* http://p.loussouarn.free.fr
* 20/04/2014: Support for MEGA added
* 22/12/2014: Support for ATtiny167 added
*             Methods TinyPinChange_Edge(), TinyPinChange_RisingEdge(), TinyPinChange_FallingEdge() added
*             TinyPinChange_GetPinEvent() replaced with TinyPinChange_GetPortEvent()
*             TinyPinChange_GetPinCurSt() replaced with TinyPinChange_GetCurPortSt()
* 01/02/2015: Fix a bug on TinyPinChange_FallingEdge() method
* 15/05/2015: Support for ATmega32U4 (Leonardo, micro, pro micro) added
*             INT0, INT1, INT2, INT3 used as emulated Pin Change Interrupt
* 15/05/2015: Fix a bug for the emulated Pin Change Interrupt pins (0, 1, 2 and 3) for ATmega32U4 (Leonardo, micro, pro micro)
* 17/12/2018: TinyPinChange_UnregisterIsr() method added
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <inttypes.h>

#ifdef __AVR_ATtinyX5__
#undef __AVR_ATtinyX5__
#endif

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define __AVR_ATtinyX5__
#endif

#ifdef __AVR_ATtinyX4__
#undef __AVR_ATtinyX4__
#endif

#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
#define __AVR_ATtinyX4__
#endif

#ifdef __AVR_ATtinyX5__
/* ATtinyX5 */
#define PIN_CHG_PORT_NB			1
#define DigitalPinToPortIdx(p)	0
#define PC_PIN0					PINB
#define PC_PCMSK0				PCMSK
#else
#ifdef __AVR_ATtinyX4__
/* ATtinyX4 */
#define PIN_CHG_PORT_NB			2
#define DigitalPinToPortIdx(p)   ((digitalPinToPCICRbit(p)==PCIE0) ?0:1)
#define PC_PIN0					PINA
#define PC_PCMSK0				PCMSK0
#define PC_PIN1					PINB
#define PC_PCMSK1				PCMSK1
#else
#if defined(__AVR_ATmega2560__)
/* MEGA */
#define PIN_CHG_PORT_NB			3
#define DigitalPinToPortIdx(p)  ((((p) >= A8) && ((p) <= A15)) ? (2) : ((((p) >= 50) && ((p) <= 53)) ? (0) : ((((p) >= 10) && ((p) <= 13)) ? (0) : (1))))
#define PC_PIN0					PINB
#define PC_PCMSK0				PCMSK0
#define PC_PIN1					PINJ
#define PC_PCMSK1				PCMSK1
#define PC_PIN2					PINK
#define PC_PCMSK2				PCMSK2
#else
#if defined(__AVR_ATtiny167__)
/* ATtiny167 */
#define PIN_CHG_PORT_NB			2
#define DigitalPinToPortIdx(p)  (((p) >= 5 && (p) <= 12) ? (0) : (1))
#define PC_PIN0					PINA
#define PC_PCMSK0				PCMSK0
#define PC_PIN1					PINB
#define PC_PCMSK1				PCMSK1
#else
#if defined(__AVR_ATmega32U4__)
/* Leonardo, micro, pro micro */
#define PIN_CHG_PORT_NB			2 /* The second one is emulated using external Interrupts */
#define DigitalPinToPortIdx(p)  ((((p) >= 8 && (p) <= 11) || ((p) >= 14 && (p) <= 17)) ? (0) : (1))
#define PC_PIN0					PINB
#define PC_PCMSK0				PCMSK0
#define PC_PIN1					PIND
#define PC_PCMSK1				0x0F /* INT0, INT1, INT2, INT3 used as emulated Pin Change Interrupts */
#else
/* UNO */
#define PIN_CHG_PORT_NB			3
#define DigitalPinToPortIdx(p)  (((p) <= 7) ? (2) : (((p) <= 13) ? (0) : (((p) <= 21) ? (1) : (0))))
#define PC_PIN0					PINB
#define PC_PCMSK0				PCMSK0
#define PC_PIN1					PINC
#define PC_PCMSK1				PCMSK1
#define PC_PIN2					PIND
#define PC_PCMSK2				PCMSK2
#endif
#endif
#endif
#endif
#endif

void    TinyPinChange_Init(void);
int8_t  TinyPinChange_RegisterIsr(uint8_t Pin, void(*Isr)(void));
uint8_t TinyPinChange_UnregisterIsr(uint8_t Pin, void(*Isr)(void));
void    TinyPinChange_EnablePin(uint8_t Pin);
void    TinyPinChange_DisablePin(uint8_t Pin);
uint8_t TinyPinChange_GetPortEvent(uint8_t VirtualPortIdx);
uint8_t TinyPinChange_GetCurPortSt(uint8_t VirtualPortIdx);
#if defined(__AVR_ATmega32U4__)
#define TinyPinChange_PinToMsk(Pin)                     (((Pin) <= 3)?(_BV(digitalPinToInterrupt((Pin)))):(_BV(digitalPinToPCMSKbit((Pin)))))
#else
#define TinyPinChange_PinToMsk(Pin)                     _BV(digitalPinToPCMSKbit((Pin)))
#endif
#define TinyPinChange_Edge(VirtualPortIdx, Pin)         ( TinyPinChange_GetPortEvent((VirtualPortIdx)) & TinyPinChange_PinToMsk((Pin)) )
#define TinyPinChange_RisingEdge(VirtualPortIdx, Pin)   ( TinyPinChange_Edge(VirtualPortIdx, Pin) &   TinyPinChange_GetCurPortSt((VirtualPortIdx))  ) 
#define TinyPinChange_FallingEdge(VirtualPortIdx, Pin)  ( TinyPinChange_Edge(VirtualPortIdx, Pin) & (~TinyPinChange_GetCurPortSt((VirtualPortIdx))) )

/*******************************************************/
/* Application Programming Interface (API) en Francais */
/*******************************************************/

/*      Methodes en Francais                           	 English native methods */
#define TinyPinChange_EnregistreFonctionInterruption		TinyPinChange_RegisterIsr
#define TinyPinChange_DesenregistreFonctionInterruption		TinyPinChange_UnregisterIsr
#define TinyPinChange_ActiveBroche				TinyPinChange_EnablePin
#define TinyPinChange_DesactiveBroche				TinyPinChange_DisablePin
#define TinyPinChange_RetourneEvenemenPort			TinyPinChange_GetPortEvent
#define TinyPinChange_RetourneEtatCourantPort			TinyPinChange_GetCurPortSt
#define TinyPinChange_MasqueDeBroche				TinyPinChange_PinToMsk
#define TinyPinChange_Front					TinyPinChange_Edge
#define TinyPinChange_FrontMontant				TinyPinChange_RisingEdge
#define TinyPinChange_FrontDescendant				TinyPinChange_FallingEdge

#endif
