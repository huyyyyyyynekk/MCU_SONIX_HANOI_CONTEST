#include <SN32F400.h>
#include <SN32F400_Def.h>
#include "..\Driver\GPIO.h"
#include "..\Driver\I2C.h"
#include "..\Driver\WDT.h"
#include "..\Driver\CT16B0.h"
#include "..\Driver\Utility.h"
#include "..\Module\EEPROM.h"
#include "..\Module\Segment.h"
#include "..\Driver\SysTick.h"

#ifndef SN32F407
	#error Please install SONiX.SN32F4_DFP.0.0.18.pack or version >= 0.0.18
#endif
#define PKG SN32F407

#define	EEPROM_WRITE_ADDR			0xa0
#define	EEPROM_READ_ADDR			0xa1	
#define	SEG_A		0x01
#define	SEG_B		0x02
#define	SEG_C		0x04
#define	SEG_D		0x08
#define	SEG_E		0x10
#define	SEG_F		0x20
#define	SEG_G		0x40
#define	SEG_H		0x80
#define PITCH_BUTTON 1
#define PITCH_DATE	 1000	
#define PITCH_TIMEOUT 500

void NotPinOut_GPIO_init(void);
__attribute__((noreturn)) void HardFault_Handler(void);
void PFPA_Init(void);
void update_segment (int h, int m);
void Buzzer(uint16_t pitch, uint8_t Mode);
void Read_EEPROM (void);
	
volatile uint16_t Timeout = 0;
volatile uint8_t Timeout_Flag = 0;
volatile uint8_t update_1m_flag = 0;
volatile uint32_t Timer_Debounce_SW3 = 0;
volatile uint8_t Blink_HH = 0;
volatile uint8_t Blink_MM = 0;
volatile uint16_t Buzzer_03 = 0;
volatile uint8_t Buzzer_03_Flag = 0;
volatile uint8_t Buzzer_5_Flag = 0;
volatile uint8_t Buzzer_5_Flag_tmp = 0;
uint16_t minute = 0;
uint16_t hour = 0;
int Mode_Normal = 0;
int Mode_Change_Time_Hour = 0;
int Mode_Change_Time_Minute = 0;
int Mode_Change_Time_Hour_Date = 0;
int Mode_Change_Time_Minute_Date = 0;
int sw3_counter = 0;
int sw3_current_state = 1;
int sw3_last_state = 1;
int sw16_counter = 0;
int sw16_current_state = 1;
int sw16_last_state = 1;
int sw6_counter = 0;
int sw6_current_state = 1;
int sw10_counter = 0;
int sw10_current_state = 1;
uint8_t Hour_Change_Time = 0;
uint8_t Minute_Change_Time = 0;
uint8_t Hour_Change_Time_Date = 0;
uint8_t Minute_Change_Time_Date = 0;
uint8_t EEPROM_Data[3];
uint8_t Check_Date_Hour;
uint8_t Check_Date_Minute;
uint8_t sw3_val;
uint8_t sw6_val;
uint8_t sw10_val;
uint8_t sw16_val;
int check = 0;
int button_check = 0;
int button_check_date = 0;

void update_segment(int h, int m) {
    int h_tmp_1 = h / 10;
    int h_tmp_2 = h % 10;
    
    int m_tmp_1 = m / 10;
    int m_tmp_2 = m % 10;
    
    segment_buff[0] = SEGMENT_TABLE[h_tmp_1];   
    segment_buff[1] = SEGMENT_TABLE[h_tmp_2];  
		segment_buff[1] |= 0x80;
    segment_buff[2] = SEGMENT_TABLE[m_tmp_1];   
    segment_buff[3] = SEGMENT_TABLE[m_tmp_2];  
}

void Buzzer(uint16_t pitch, uint8_t Mode){
	if (pitch > 0){
		SN_CT16B0->MR9 = 12000000UL / pitch;				
		SN_CT16B0->MR0 = SN_CT16B0->MR9 >> Mode;					// 50% duty cycle
		
		SN_CT16B0->TMRCTRL = 0;												//START TIMER
		SN_CT16B0->TMRCTRL = 1;												//START TIMER
	}
	else {
		SN_CT16B0->MR0 = 0;														//disable buzzer
	}
}

void Read_EEPROM(void){
    uint8_t h_m_tmp[3];
    eeprom_read(EEPROM_READ_ADDR, 0x00, h_m_tmp, 3);
		if (h_m_tmp[2] == 0xAA){
			check = 1;
		}
		else {
			check = 0;
		}
    if(h_m_tmp[0] <= 23)
        Check_Date_Hour = h_m_tmp[0];
    else
        Check_Date_Hour = 0;
    if(h_m_tmp[1] <= 59)
        Check_Date_Minute = h_m_tmp[1];
    else
        Check_Date_Minute = 0;
}

int main(void) {
    SystemInit();
		SystemCoreClockUpdate();
		PFPA_Init();
		NotPinOut_GPIO_init();
		SysTick_Init();
		SN_SYS0->EXRSTCTRL_b.RESETDIS = 0;
		GPIO_Init();				
		CT16B0_Init();
		SN_PFPA->CT16B0_b.PWM0 = 1;
		I2C0_Init();
		update_segment(0,0);
		Read_EEPROM();
	
		SN_GPIO2->MODE &= ~((3 << 8) | (3 << 14));  // Xóa bit Mode -> Input cho P2.4 và P2.7
		SN_GPIO2->CFG  &= ~((3 << 8) | (3 << 14));  // Xóa cu
		SN_GPIO2->CFG  |=  ((2 << 8) | (2 << 14));  // Set giá tr? 2 (10b) -> Pull-up cho P2.4 và P2.7
		
		SN_GPIO1->MODE |= ((1 << 4) | (1 << 5) | (1 << 7));  //output

		SN_GPIO1->DATA |= ((1 << 4) | (1 << 5) | (1 << 7)); // 1
		
		//Led D6 - P3.8 output 
		SN_GPIO3->MODE |= (1 << 8);
    SN_GPIO3->DATA |= (1 << 8);
    while(1) {
			__WDT_FEED_VALUE;
				if (update_1m_flag == 1){
					update_1m_flag = 0;
					minute++;
				if (minute > 59){
					minute = 0;
					hour++;
					if (hour > 23){
						hour = 0;
					}
				}
				update_segment(hour, minute);
				}
				SN_GPIO3->DATA |= (1 << 8);
			
			//Quét Hàng 0 - SW3 và SW6 
			SN_GPIO1->DATA &= ~(1 << 4); // Kéo ROW0 xuong LOW
			UT_DelayNx10us(2); 
			sw3_val = (SN_GPIO2->DATA & (1 << 4)) ? 1 : 0; 
			sw6_val = (SN_GPIO2->DATA & (1 << 7)) ? 1 : 0; 
			SN_GPIO1->DATA |= (1 << 4);  // Keo ROW0 lên HIGH lai

			//Quét Hàng 1 - SW10 
			SN_GPIO1->DATA &= ~(1 << 5); // Kéo ROW1 xuong LOW
			UT_DelayNx10us(2);
			sw10_val = (SN_GPIO2->DATA & (1 << 7)) ? 1 : 0; 
			SN_GPIO1->DATA |= (1 << 5);  // Kéo ROW1 lên HIGH lai

			//Quét Hàng 3 - SW16 ---
			SN_GPIO1->DATA &= ~(1 << 7); // Kéo ROW3 xuong LOW
			UT_DelayNx10us(2);
			sw16_val = (SN_GPIO2->DATA & (1 << 4)) ? 1 : 0; 
			SN_GPIO1->DATA |= (1 << 7);  // Kéo ROW3 lên HIGH lai
			
			//Bam nut sw3
			if (sw3_val == sw3_current_state){
				sw3_counter = 0;
			}
			else {
				sw3_counter++;
				if (sw3_counter >= 100){
					sw3_current_state = sw3_val;
					sw3_counter = 0;
					if (sw3_current_state == 0 && sw3_last_state == 1){
						Buzzer_03 = 300;
						if (Mode_Normal == 0){
							Hour_Change_Time = hour;
							Minute_Change_Time = minute;
							Mode_Change_Time_Hour = 1;
							Mode_Change_Time_Minute = 0;
							button_check = 0;
							Mode_Normal = 1;
						}
						else if (Mode_Normal == 1 && Mode_Change_Time_Hour == 1 && Mode_Change_Time_Minute == 0){
								Minute_Change_Time = minute;
								hour = Hour_Change_Time;
								button_check = 1;
								Mode_Change_Time_Hour = 1;
								Mode_Change_Time_Minute = 1;
						}
						else if (Mode_Normal == 1 && Mode_Change_Time_Hour == 1 && Mode_Change_Time_Minute == 1){
							Mode_Normal = 0;
							Mode_Change_Time_Hour = 0;
							Mode_Change_Time_Minute = 0;
							hour = Hour_Change_Time;
							minute = Minute_Change_Time;
							update_segment(hour,minute);
						}
					}
				}
			}
			sw3_last_state = sw3_current_state;
			//Bam nut sw3
			
			//Blink-HH-MM-sw3
			if(Mode_Normal == 1 && Mode_Change_Time_Hour == 1 && Mode_Change_Time_Minute == 0){
				if (Blink_HH == 1){
					segment_buff[0] = SEGMENT_TABLE[Hour_Change_Time / 10];   
					segment_buff[1] = SEGMENT_TABLE[Hour_Change_Time % 10];
				}
				else {
					segment_buff[0] = 0x00;   
					segment_buff[1] = 0x00;
				}
				segment_buff[2] = SEGMENT_TABLE[Minute_Change_Time / 10];
        segment_buff[3] = SEGMENT_TABLE[Minute_Change_Time % 10];
				segment_buff[1] |= 0x80;
			}
			else  if (Mode_Normal == 1 && Mode_Change_Time_Hour == 1 && Mode_Change_Time_Minute == 1)	{
				if (Blink_MM == 1){
					segment_buff[2] = SEGMENT_TABLE[Minute_Change_Time / 10];   
					segment_buff[3] = SEGMENT_TABLE[Minute_Change_Time % 10];
				}
				else {
					segment_buff[2] = 0x00;   
					segment_buff[3] = 0x00;
				}	
				segment_buff[0] = SEGMENT_TABLE[Hour_Change_Time / 10];
        segment_buff[1] = SEGMENT_TABLE[Hour_Change_Time % 10];
				segment_buff[1] |= 0x80;
			}
			//Blink-HH-MM-sw3
			
			//Bam nut sw16
			if (sw16_val == sw16_current_state){
				sw16_counter = 0;
			}
			else {
				sw16_counter++;
				if (sw16_counter >= 100){
					sw16_current_state = sw16_val;
					sw16_counter = 0;
					if (sw16_current_state == 0 && sw16_last_state == 1){
						Buzzer_03 = 300;
						if (Mode_Normal == 0){
							Mode_Change_Time_Hour_Date = 1;
							Mode_Change_Time_Minute_Date = 0;
							button_check_date = 0;
							Mode_Normal = 1;
						}
						else if (Mode_Normal == 1 && Mode_Change_Time_Hour_Date == 1 && Mode_Change_Time_Minute_Date == 0){
							button_check_date = 1;
							Mode_Change_Time_Hour_Date = 1;
							Mode_Change_Time_Minute_Date = 1;
						}
						else if (Mode_Normal == 1 && Mode_Change_Time_Hour_Date == 1 && Mode_Change_Time_Minute_Date == 1){
							Mode_Normal = 0;
							Mode_Change_Time_Hour_Date = 0;
							Mode_Change_Time_Minute_Date = 0;
							EEPROM_Data[0] = Hour_Change_Time_Date;
							EEPROM_Data[1] = Minute_Change_Time_Date;
							EEPROM_Data[2] = 0xAA;
							eeprom_write(EEPROM_WRITE_ADDR,0x00,EEPROM_Data,3);
							UT_DelayNx10us(1000);
							Read_EEPROM();
							update_segment(hour,minute);
						}
					}
				}
			}
			sw16_last_state = sw16_current_state;
			//Bam nut sw16
			
			//Blink-HH-MM-sw16
			if(Mode_Normal == 1 && Mode_Change_Time_Hour_Date == 1 && Mode_Change_Time_Minute_Date == 0){
				if (Blink_HH == 1){
					segment_buff[0] = SEGMENT_TABLE[Hour_Change_Time_Date / 10];   
					segment_buff[1] = SEGMENT_TABLE[Hour_Change_Time_Date % 10];
					SN_GPIO3->DATA &= ~(1 << 8);
				}
				else {
					segment_buff[0] = 0x00;   
					segment_buff[1] = 0x00;
					SN_GPIO3->DATA |= (1 << 8);
				}
				segment_buff[2] = SEGMENT_TABLE[Minute_Change_Time_Date / 10];
        segment_buff[3] = SEGMENT_TABLE[Minute_Change_Time_Date % 10];
				segment_buff[1] |= 0x80;
			}
			else  if (Mode_Normal == 1 && Mode_Change_Time_Hour_Date == 1 && Mode_Change_Time_Minute_Date == 1)	{
				if (Blink_MM == 1){
					segment_buff[2] = SEGMENT_TABLE[Minute_Change_Time_Date / 10];   
					segment_buff[3] = SEGMENT_TABLE[Minute_Change_Time_Date % 10];
					SN_GPIO3->DATA &= ~(1 << 8);
				}
				else {
					segment_buff[2] = 0x00;   
					segment_buff[3] = 0x00;
					SN_GPIO3->DATA |= (1 << 8);
				}	
				segment_buff[0] = SEGMENT_TABLE[Hour_Change_Time_Date / 10];
        segment_buff[1] = SEGMENT_TABLE[Hour_Change_Time_Date % 10];
				segment_buff[1] |= 0x80;
			}
			//Blink-HH-MM-sw16
			
			//Bam nut sw6
			if (sw6_val == sw6_current_state){
				sw6_counter = 0;
			}
			else {
				sw6_counter++;
				if (sw6_counter >= 100){
					sw6_current_state = sw6_val;
					sw6_counter = 0;
					if (sw6_current_state == 0){
						Buzzer_03 = 300;
						if (Mode_Change_Time_Hour == 1 && Mode_Change_Time_Minute == 0 && Mode_Normal == 1){
							Hour_Change_Time++;
							if (Hour_Change_Time > 23){
								Hour_Change_Time = 0;
							}
						}
						else if (Mode_Change_Time_Hour_Date == 1 && Mode_Change_Time_Minute_Date == 0 && Mode_Normal == 1){
							Hour_Change_Time_Date++;
							if (Hour_Change_Time_Date > 23){
								Hour_Change_Time_Date = 0;
							}
						}
						else if (Mode_Change_Time_Minute == 1 && Mode_Change_Time_Hour == 1 && Mode_Normal == 1){
							Minute_Change_Time++;
							if (Minute_Change_Time > 59){
								Minute_Change_Time = 0;
							} 
						}
						else if (Mode_Change_Time_Minute_Date == 1 && Mode_Change_Time_Hour_Date == 1 && Mode_Normal == 1){
							Minute_Change_Time_Date++;
							if (Minute_Change_Time_Date > 59){
								Minute_Change_Time_Date = 0;
							}
						}
					}
				}
			}
			//Bam nut sw6
			
			//Bam nut sw10
			if (sw10_val == sw10_current_state){
				sw10_counter = 0;
			}
			else {
				sw10_counter++;
				if (sw10_counter >= 100){
					sw10_current_state = sw10_val;
					sw10_counter = 0;
					if (sw10_current_state == 0){
						Buzzer_03 = 300;
						if (Mode_Change_Time_Hour == 1 && Mode_Change_Time_Minute == 0 && Mode_Normal == 1){
							if (Hour_Change_Time > 0){
								Hour_Change_Time--;
							}
							else if (Hour_Change_Time <= 0){
								Hour_Change_Time = 23;
							}
						}
						else if (Mode_Change_Time_Hour_Date == 1 && Mode_Change_Time_Minute_Date == 0 && Mode_Normal == 1){
							if (Hour_Change_Time_Date > 0){
								Hour_Change_Time_Date--;
							}
							else if (Hour_Change_Time_Date <= 0){
								Hour_Change_Time_Date = 23;
							}
						}
						else if (Mode_Change_Time_Minute == 1 && Mode_Change_Time_Hour == 1 && Mode_Normal == 1){
							if (Minute_Change_Time > 0){
								Minute_Change_Time--;
							}
							else if (Minute_Change_Time <= 0){
								Minute_Change_Time = 59;
							} 
						}
						else if (Mode_Change_Time_Minute_Date == 1 && Mode_Change_Time_Hour_Date == 1 && Mode_Normal == 1){
							if (Minute_Change_Time_Date > 0){
								Minute_Change_Time_Date--;
							}
							else if (Minute_Change_Time_Date <= 0){
								Minute_Change_Time_Date = 59;
							}
						}
					}
				}
			}
			//Bam nut sw10
			
			//Buzzer-Timeout-QuetLed
			if (sw3_val == 0 || sw6_val == 0 || sw10_val == 0 || sw16_val == 0) {
					Timeout = 30000; 
			}
			if (Mode_Normal == 1 && Timeout_Flag == 1 && (button_check == 1 || button_check_date == 1)) {
				Buzzer_03 = 300;
				Mode_Normal = 0;
				Mode_Change_Time_Hour = 0;
				Mode_Change_Time_Minute = 0;
				Mode_Change_Time_Hour_Date = 0;
				Mode_Change_Time_Minute_Date = 0;
				hour = Hour_Change_Time;
				minute = Minute_Change_Time;
				update_segment(hour,minute);
			}
			if (Mode_Normal == 1 && Timeout_Flag == 1 && (button_check == 0 || button_check_date == 0)) {
				Buzzer_03 = 300;
				Mode_Normal = 0;
				Mode_Change_Time_Hour = 0;
				Mode_Change_Time_Minute = 0;
				Mode_Change_Time_Hour_Date = 0;
				Mode_Change_Time_Minute_Date = 0;
				update_segment(hour,minute);
			}
				
			if (Mode_Normal == 0) {
				if (hour == Check_Date_Hour && minute == Check_Date_Minute && check == 1) {
						if (Buzzer_5_Flag_tmp == 0) {
								Buzzer_5_Flag = 1;     
								Buzzer_5_Flag_tmp = 1; 
						}
				} 
				else {
						Buzzer_5_Flag_tmp = 0; 
				}
			}
			if (Buzzer_03_Flag == 1) {
					Buzzer(PITCH_BUTTON, 6);
			} 
			else if (Buzzer_5_Flag == 1) {
					Buzzer(PITCH_DATE, 1);
			} 
			else {
					SN_CT16B0->MR0 = 0;
			}
			Digital_Scan();
			//Buzzer-Timeout-QuetLed
    }
}

/*****************************************************************************
* Function		: NotPinOut_GPIO_init
*****************************************************************************/
void NotPinOut_GPIO_init(void)
{
#if (PKG == SN32F405)
	//set P0.4, P0.6, P0.7 to input pull-up
	SN_GPIO0->CFG = 0x00A008AA;
	//set P1.4 ~ P1.12 to input pull-up
	SN_GPIO1->CFG = 0x000000AA;
	//set P3.8 ~ P3.11 to input pull-up
	SN_GPIO3->CFG = 0x0002AAAA;
#elif (PKG == SN32F403)
	//set P0.4 ~ P0.7 to input pull-up
	SN_GPIO0->CFG = 0x00A000AA;
	//set P1.4 ~ P1.12 to input pull-up
	SN_GPIO1->CFG = 0x000000AA;
	//set P2.5 ~ P2.6, P2.10 to input pull-up
	SN_GPIO2->CFG = 0x000A82AA;
	//set P3.0, P3.8 ~ P3.13 to input pull-up
	SN_GPIO3->CFG = 0x0000AAA8;
#endif
}

/*****************************************************************************
* Function		: HardFault_Handler
* Description	: ISR of Hard fault interrupt
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void HardFault_Handler(void)
{
	NVIC_SystemReset();
}
