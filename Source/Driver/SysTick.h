#ifndef __SN32F400_SYSTICK_H
#define __SN32F400_SYSTICK_H

/*_____ I N C L U D E S ____________________________________________________*/
#include <SN32F400.h>#ifndef __SN32F400_SYSTICK_H
#define __SN32F400_SYSTICK_H

/*_____ I N C L U D E S ____________________________________________________*/
#include <SN32F400.h>
#include <SN32F400_Def.h>

/*_____ D E F I N I T I O N S ______________________________________________*/
#define	SYSTICK_IRQ		INTERRUPT_METHOD		//INTERRUPT_METHOD:	Enable SysTick timer and interrupt
																					//POLLING_METHOD: 	Enable SysTick timer ONLY
extern volatile uint16_t Timeout; 
extern volatile uint8_t Timeout_Flag;
extern volatile uint8_t update_1m_flag;
extern volatile uint32_t Timer_Debounce_SW3;
extern volatile uint8_t Blink_HH;
extern volatile uint8_t Blink_MM;
extern volatile uint16_t Buzzer_03;
extern volatile uint8_t Buzzer_03_Flag;
extern volatile uint8_t Buzzer_5_Flag_tmp;
extern volatile uint8_t Buzzer_5_Flag;
/*_____ M A C R O S ________________________________________________________*/
#define	__SYSTICK_CLEAR_COUNTER_AND_FLAG	SysTick->CTRL |= SysTick_CTRL_COUNTFLAG_Msk;

/*_____ D E C L A R A T I O N S ____________________________________________*/
void	SysTick_Init(void);
void	SysTick_ConfigTime(uint32_t);
void SysTick_Handler(void);
#endif	/*__SN32F400_SYSTICK_H*/
