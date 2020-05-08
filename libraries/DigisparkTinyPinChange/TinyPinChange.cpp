/********************************************************************************/
/* PROJECT:   All based on ATtinyX5, ATtinyX4, Atiny167, ATmega328P, ATmega2560,*/
/*            ATmega32U4.                                                       */
/* MODULE:    TinyPinChange                                                     */
/* VERSION:   1.5 (31/10/2015)                                                  */
/* DATE:      30/01/2011                                                        */
/* TARGET:    ATtinyX5, ATtinyX4, ATtiny167, ATmega328P, ATmega2560             */
/* COMPILER:  WinAvr, avr-gcc, avr-g++                                          */
/* IDE:       ARDUINO, AVR Studio 4                                             */
/* PROGRAMER: AVR-JTAG-ICE MKII, ARDUINO IDE                                    */
/* AUTHOR:    P.LOUSSOUARN     (P.Loussouarn: http://p.loussouarn.free.fr)      */
/********************************************************************************/
#include <TinyPinChange.h>
#include <avr/interrupt.h>

/*************************************************************************
								MACROS
*************************************************************************/

#if defined(__AVR_ATmega32U4__)
#define PIN_CHANGE_HANDLER_MAX_NB	4	/* ISR max number Pin Change ISR can handle per port (INT0, INT1, INT2, INT3 for ATmega32U4) */
#else
#define PIN_CHANGE_HANDLER_MAX_NB	3	/* ISR max number Pin Change ISR can handle per port */
#endif

/*************************************************************************
							GLOBAL VARIABLES
*************************************************************************/
typedef struct{

	void			(*Isr[PIN_CHANGE_HANDLER_MAX_NB])();
	uint8_t			LoadedIsrNb;
	uint8_t			Event;
	uint8_t			PinPrev;
	uint8_t			PinCur;
}PinChangeSt_t;

typedef struct{
	PinChangeSt_t Port[PIN_CHG_PORT_NB];
}PinChangePortSt_t;

static volatile PinChangePortSt_t PinChange;

#if defined(__AVR_ATmega32U4__)
static void ExtInt0AsEmulatedPinChangeInt(void);
static void ExtInt1AsEmulatedPinChangeInt(void);
static void ExtInt2AsEmulatedPinChangeInt(void);
static void ExtInt3AsEmulatedPinChangeInt(void);
#endif

/*************************************************************************
							INTERRUPT SUB-ROUTINE
*************************************************************************/
#define DECLARE_PIN_CHANGE_ISR(VirtualPortIdx)                                                                             \
ISR(PCINT##VirtualPortIdx##_vect)                                                                                          \
{                                                                                                                          \
  uint8_t Idx;                                                                                                             \
  PinChange.Port[VirtualPortIdx].PinCur  = (PC_PIN##VirtualPortIdx) & (PC_PCMSK##VirtualPortIdx);                          \
  PinChange.Port[VirtualPortIdx].Event   = PinChange.Port[VirtualPortIdx].PinPrev ^ PinChange.Port[VirtualPortIdx].PinCur; \
  PinChange.Port[VirtualPortIdx].PinPrev = PinChange.Port[VirtualPortIdx].PinCur;                                          \
  for(Idx = 0; Idx < PinChange.Port[VirtualPortIdx].LoadedIsrNb; Idx++)                                                    \
  {                                                                                                                        \
    PinChange.Port[VirtualPortIdx].Isr[Idx]();                                                                             \
  }                                                                                                                        \
}

DECLARE_PIN_CHANGE_ISR(0)

#if defined(__AVR_ATtinyX4__) || defined(__AVR_ATtiny167__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
DECLARE_PIN_CHANGE_ISR(1)
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
DECLARE_PIN_CHANGE_ISR(2)
#endif

/*************************************************************************
			PUBLIC FUNCTIONS
*************************************************************************/

/*********************************************************************
	PinChange Initialization Function
Input:
	Void
Output:
	Void
*********************************************************************/
void TinyPinChange_Init(void)
{
/* ATtinyX5, ATtiny167, ATtinyX4, UNO, MEGA */
	PinChange.Port[0].PinCur = PC_PIN0 & PC_PCMSK0;//PINB for ATtinyX5, UNO or MEGA, PINA for ATtinyX4 or ATtiny167, PINB for ATmega32U4 (Leonardo, micro, pro micro)
	PinChange.Port[0].PinPrev = PinChange.Port[0].PinCur;
#if defined(__AVR_ATtinyX4__) || defined(__AVR_ATtiny167__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
/* ATtinyX4, ATtiny167, UNO or MEGA */
	PinChange.Port[1].PinCur = PC_PIN1 & PC_PCMSK1;//PINB for for ATtinyX4 or ATtiny167, PINC for UNO, PINJ for MEGA
	PinChange.Port[1].PinPrev = PinChange.Port[1].PinCur;
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
/* UNO or MEGA */
	PinChange.Port[2].PinCur = PC_PIN2 & PC_PCMSK2;//PIND for UNO, PINK for MEGA
	PinChange.Port[2].PinPrev = PinChange.Port[2].PinCur;
#endif
#if defined(__AVR_ATmega32U4__)
/* ATmega32U4 (Leonardo, micro, pro micro) */
	PinChange.Port[1].PinCur = PC_PIN1 & PC_PCMSK1;//PIND for for ATmega32U4 (Leonardo, micro, pro micro)
	PinChange.Port[1].PinPrev = PinChange.Port[1].PinCur;
#endif
}

/*********************************************************************
	PinChange RegisterIsr Function
Input:
    Pin: Pin number
    Isr: Pointer on a PinChange Function
Output:
	The associated VirtualPortIdx (0 to 2)
	< 0 in case of failure
*********************************************************************/
int8_t TinyPinChange_RegisterIsr(uint8_t Pin, void(*Isr)())
{
int8_t IsrIdx, PortIdx, AlreadyLoaded = 0;

	PortIdx = DigitalPinToPortIdx(Pin);

#if defined(__AVR_ATmega32U4__)
	if(Pin >= 0 && Pin <= 3)
	{
	  /* INT0, INT1, INT2, INT3 used as emulated Pin Change Interrupt */
	  switch(digitalPinToInterrupt(Pin))
	  {
	    case 0: /* Ext INT0 */
	    PinChange.Port[PortIdx].Isr[0] = Isr;
	    attachInterrupt(0, ExtInt0AsEmulatedPinChangeInt, CHANGE);
	    break;
	    
	    case 1: /* Ext INT1 */
	    PinChange.Port[PortIdx].Isr[1] = Isr;
	    attachInterrupt(1, ExtInt1AsEmulatedPinChangeInt, CHANGE);
	    break;
	    
	    case 2: /* Ext INT2 */
	    PinChange.Port[PortIdx].Isr[2] = Isr;
	    attachInterrupt(2, ExtInt2AsEmulatedPinChangeInt, CHANGE);
	    break;
	    
	    case 3: /* Ext INT3 */
	    PinChange.Port[PortIdx].Isr[3] = Isr;
	    attachInterrupt(3, ExtInt3AsEmulatedPinChangeInt, CHANGE);
	    break;
	  }
	}
	else
	{
#endif
	  for(IsrIdx = 0; IsrIdx < PIN_CHANGE_HANDLER_MAX_NB; IsrIdx++)
	  {
	    if(PinChange.Port[PortIdx].Isr[IsrIdx] == Isr)
	    {
	      AlreadyLoaded = 1;
	      break; /* Already loaded */
	    }
	  }
	  
	  if(!AlreadyLoaded)
	  {
	    if(PinChange.Port[PortIdx].LoadedIsrNb < PIN_CHANGE_HANDLER_MAX_NB)
	    {
	      /* Not aready loaded: load it */
	      PinChange.Port[PortIdx].Isr[PinChange.Port[PortIdx].LoadedIsrNb] = Isr;
	      PinChange.Port[PortIdx].LoadedIsrNb++;
	    }
	    else PortIdx = -1; /* Failure */
	  }
#if defined(__AVR_ATmega32U4__)
	}
#endif
	return(PortIdx);
}

/*********************************************************************
    PinChange UnregisterIsr Function
Input:
    Pin: Pin number
    Isr: Pointer on a PinChange Function
Output:
    0: Unregister failure
    1: OK, Isr unregistered
*********************************************************************/
uint8_t TinyPinChange_UnregisterIsr(uint8_t Pin, void(*Isr)())
{
    int8_t  IsrIdx, PortIdx, FoundIdx = -1;
    uint8_t Ret = 0;
    
    PortIdx = DigitalPinToPortIdx(Pin);
#if defined(__AVR_ATmega32U4__)
	if(Pin >= 0 && Pin <= 3)
	{
#warning TO DO: implement TinyPinChange_UnregisterIsr ofr pin 0-3 of ATmega32U4
	  /* INT0, INT1, INT2, INT3 used as emulated Pin Change Interrupt */
	  switch(digitalPinToInterrupt(Pin))
	  {
	    case 0: /* Ext INT0 */
//	    PinChange.Port[PortIdx].Isr[0] = Isr;
//	    attachInterrupt(0, ExtInt0AsEmulatedPinChangeInt, CHANGE);
	    break;
	    
	    case 1: /* Ext INT1 */
//	    PinChange.Port[PortIdx].Isr[1] = Isr;
//	    attachInterrupt(1, ExtInt1AsEmulatedPinChangeInt, CHANGE);
	    break;
	    
	    case 2: /* Ext INT2 */
//	    PinChange.Port[PortIdx].Isr[2] = Isr;
//	    attachInterrupt(2, ExtInt2AsEmulatedPinChangeInt, CHANGE);
	    break;
	    
	    case 3: /* Ext INT3 */
//	    PinChange.Port[PortIdx].Isr[3] = Isr;
//	    attachInterrupt(3, ExtInt3AsEmulatedPinChangeInt, CHANGE);
	    break;
	  }
	}
	else
	{
#endif
    if(PortIdx >= 0)
    {
        for(IsrIdx = 0; IsrIdx < PinChange.Port[PortIdx].LoadedIsrNb; IsrIdx++)
        {
            if(PinChange.Port[PortIdx].Isr[IsrIdx] == Isr)
            {
                FoundIdx = IsrIdx;
                break; /* Already loaded -> can be removed */
            }
        }
        if(FoundIdx >= 0)
        {
            if(FoundIdx != (PinChange.Port[PortIdx].LoadedIsrNb - 1))
            {
                /* Not the last ISR loaded -> SHALL shift subsequent ISR to the left */
                for(uint8_t Idx = FoundIdx; Idx < (PinChange.Port[PortIdx].LoadedIsrNb - 1); Idx++)
                {
                    PinChange.Port[PortIdx].Isr[Idx] = PinChange.Port[PortIdx].Isr[Idx + 1];
                }
            }
            else
            {
                /* ISR is the last one: just decrement LoadedIsrNb */
            }
            PinChange.Port[PortIdx].LoadedIsrNb--;
            Ret = 1;
        }
    }
#if defined(__AVR_ATmega32U4__)
	}
#endif
    return(Ret);
}

/*********************************************************************
	PinChange Enable Pin Function
Input:
	Pin: the Pin
Output:
	Void
*********************************************************************/
void TinyPinChange_EnablePin(uint8_t Pin)
{
#if defined(__AVR_ATmega32U4__)
    if(Pin <= 3)
    {
      EIMSK |= (1 << digitalPinToInterrupt(Pin));
    }
    else
    {
      if(digitalPinToPCICR(Pin))
      {
	*digitalPinToPCICR(Pin) |= _BV(digitalPinToPCICRbit(Pin));
	*digitalPinToPCMSK(Pin) |= _BV(digitalPinToPCMSKbit(Pin));
      }
    }
#else
    if(digitalPinToPCICR(Pin))
    {
      *digitalPinToPCICR(Pin) |= _BV(digitalPinToPCICRbit(Pin));
      *digitalPinToPCMSK(Pin) |= _BV(digitalPinToPCMSKbit(Pin));
    }
#endif
}

/*********************************************************************
	PinChange Disable Pin Function
Input:
	Pin: the Pin
Output:
	Void
*********************************************************************/
void TinyPinChange_DisablePin(uint8_t Pin)
{
#if defined(__AVR_ATmega32U4__)
    if(Pin <= 3)
    {
      EIMSK &= ~(1 << digitalPinToInterrupt(Pin));      
    }
    else
    {
      if(digitalPinToPCICR(Pin))
      {
	    *digitalPinToPCMSK(Pin) &= (_BV(digitalPinToPCMSKbit(Pin)) ^ 0xFF);
      }
    }
#else
    if(digitalPinToPCICR(Pin))
    {
	  *digitalPinToPCMSK(Pin) &= (_BV(digitalPinToPCMSKbit(Pin)) ^ 0xFF);
    }
#endif
}

/*********************************************************************
	PinChange GetPortEvent Function
Input:
	VirtualPortIdx: Index of the Port
Output:
	The bits which have changed in the port
*********************************************************************/
uint8_t TinyPinChange_GetPortEvent(uint8_t VirtualPortIdx)
{
	return(PinChange.Port[VirtualPortIdx].Event);
}

/*********************************************************************
	PinChange GetCurPortSt Function
Input:
	VirtualPortIdx: Index of the Port
Output:
	Current Status of the port
*********************************************************************/
uint8_t TinyPinChange_GetCurPortSt(uint8_t VirtualPortIdx)
{
	return(PinChange.Port[VirtualPortIdx].PinCur);
}

/*************************************************************************
			PRIVATE FUNCTIONS
*************************************************************************/
#if defined(__AVR_ATmega32U4__)
#define DECLARE_EXT_INT_AS_PIN_CHANGE_INT(ExtIntIdx)                                \
static void ExtInt##ExtIntIdx##AsEmulatedPinChangeInt(void)                         \
{                                                                                   \
  PinChange.Port[1].PinCur  = (PC_PIN1) & (PC_PCMSK1);                              \
  PinChange.Port[1].Event   = PinChange.Port[1].PinPrev ^ PinChange.Port[1].PinCur; \
  PinChange.Port[1].PinPrev = PinChange.Port[1].PinCur;                             \
  PinChange.Port[1].Isr[(ExtIntIdx)]();                                             \
}
DECLARE_EXT_INT_AS_PIN_CHANGE_INT(0)
DECLARE_EXT_INT_AS_PIN_CHANGE_INT(1)
DECLARE_EXT_INT_AS_PIN_CHANGE_INT(2)
DECLARE_EXT_INT_AS_PIN_CHANGE_INT(3)
#endif
