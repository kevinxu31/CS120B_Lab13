/*	Author: lab
 *  Partner(s) Name: Luofeng Xu
 *	Lab Section:
 *	Assignment: Lab 13  Exercise 4
 *	Exercise Description: I have an alternate solution, with two SMs control U/D and L/R, the demo video will be in my youtube channel, and here's the link for that:https://youtu.be/64-FlBYc1qs
 *				Not sure which one is suitable for the requirement, but this one will looks a little bit better.
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: Youtube URL>https://youtu.be/5NubqPY1XIA
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); } 
}


typedef struct task{
        int state;
        unsigned long period;
        unsigned long elapsedTime;
        int(*TickFct)(int);
}task;

task tasks[2];
const unsigned short tasksNum=2;
const unsigned long tasksPeriod=1;
volatile unsigned char TimerFlag = 0; 
unsigned long _avr_timer_M = 1; 
unsigned long _avr_timer_cntcurr = 0; 
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
void TimerOn() {
	TCCR1B 	= 0x0B;	
	OCR1A 	= 125;	
	TIMSK1 	= 0x02; 
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;	
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B 	= 0x00; 
}

void TimerISR() {
	unsigned char i;
	for(i=0;i<tasksNum;++i){
		if(tasks[i].elapsedTime>=tasks[i].period){
			tasks[i].state=tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime=0;
		}
		tasks[i].elapsedTime+=tasksPeriod;
	}
}
ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) { 	
		TimerISR(); 				
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void ADC_init(){
	ADCSRA |=(1<<ADEN)|(1<<ADSC)|(1<<ADATE);
}
unsigned short x1;
unsigned short x2;

enum Shift_States{adc1};
int Shift_Tick(int state){
	switch(state){
		case adc1:
			state=adc1;
			break;
		
		default:
			break;
	}
	switch(state){
		case adc1:
			Set_A2D_Pin(0x00);
                        x1=ADC;
                        Set_A2D_Pin(0x01);
                        x2=ADC;
			break;
		default:
			break;
	}
	return state;
}
unsigned char sp1=2;
unsigned char sp2=2;
enum Speed_States{speed};
int Speed_Tick(int state1){
	switch(state1){
		case speed:
			break;
		default:
			state1=speed;
			break;
	}
	switch(state1){
		case speed:
			if (((x1>600)&&(x1<=700))||((x1<500)&&(x1>=400))){
				sp1=20;
			}
			else if(((x1>700)&&(x1<=800))||((x1<400)&&(x1>=300))){
				sp1=10;
			}
			else if(((x1>800)&&(x1<=900))||((x1<300)&&(x1>=200))){
                                sp1=5;
                        }
			else if((x1>900)||(x1<200)){
                                sp1=2;
                        }
			if (((x2>600)&&(x2<=700))||((x2<500)&&(x2>=400))){
                                sp2=20;
                        }
                        else if(((x2>700)&&(x2<=800))||((x2<400)&&(x2>=300))){
                                sp2=10;
                        }
                        else if(((x2>800)&&(x2<=900))||((x2<300)&&(x2>=200))){
                                sp2=5;
                        }
                        else if((x2>900)||(x2<200)){
                                sp2=2;
                        }

			break;
		default:
			break;
	}
	return state1;

}



enum LED_States {start,init,move};
int LED_Tick(int state2) {

	static unsigned char t=0;
	static unsigned char pattern = 0x80;	
	static unsigned char row = 0xFE;  	 
						
	switch (state2) {
		case start:
			state2=init;	
			pattern=0x80;
			row=0xFE;
			t=0;
			break;
		case init:
			if((x1>=500)&&(x1<=600)&&(x2>=500)&&(x2<=600)){
				state2=init;				
			}
			else {
				t=0;
				state2=move;
			}
			
			break;
		case move:
			if((x1<=600)&&(x1>=500)&&(x2>=500)&&(x2<=600)){
				state2=init;
			}
			else{
				state2=move;
			}
			break;
		default:			
			break;
	}	
	switch (state2) {
		case start:
			break;
		case init:
			break;
		case move:
			if(t%sp1==0){
				if((x1>600)&&((x2>=500)&&(x2<=600))){
					if (pattern == 0x01) { 
						pattern = 0x01;		  			
					}
					else { 
						pattern >>= 1;
					}
				}
				else if((x1<500)&&((x2>=500)&&(x2<=600))){
					if(pattern==0x80){
						pattern=0x80;
					}
					else{
						pattern<<=1;
					}
				}
				else if((x2>600)&&((x1>=500)&&(x1<=600))){
                                        if (row == 0xFE) {
                                                row = 0xFE;
                                        }
                                        else {
                                                row=(row>>1) |0x80;
                                        }
                                }
                                else if((x2<500)&&((x1>=500)&&(x1<=600))){
                                        if(row==0xEF){
                                                row=0xEF;
                                        }
                                        else{
                                                row=(row<<1)|0x01;
                                        }
                                }
				else if((x1>600)&&(x2>600)){
					if ((pattern == 0x01)&&(row != 0xFE)) {
                                                pattern = 0x01;
						row=(row>>1) |0x80;
                                        }
					else if((pattern != 0x01)&&(row == 0xFE)){
						row=0xFE;
						pattern >>= 1;
					}
                                        else if((pattern == 0x01)&&(row == 0xFE)){
						pattern = 0x01;
						row=0xFE;
					}
					else{
                                                pattern >>= 1;
						row=(row>>1) |0x80;
                                        }

				}
				else if((x1>600)&&(x2<500)){
                                        if ((pattern == 0x01)&&(row != 0xEF)) {
                                                pattern = 0x01;
						row=(row<<1) |0x01;
                                        }
                                        else if((pattern != 0x01)&&(row == 0xEF)){
                                                row=0xEF;
						pattern >>= 1;
                                        }
                                        else if((pattern == 0x01)&&(row == 0xEF)){
                                                pattern = 0x01;
                                                row=0xEF;
                                        }
                                        else{
                                                pattern >>= 1;
                                                row=(row<<1) |0x01;
                                        }
				}
				else if((x1<500)&&(x2>600)){
                                        if ((pattern == 0x80)&&(row != 0xFE)) {
                                                pattern = 0x80;
						row=(row>>1) |0x80;
                                        }
                                        else if((pattern != 0x80)&&(row == 0xFE)){
                                                row=0xFE;
						pattern <<= 1;
                                        }
                                        else if((pattern == 0x80)&&(row == 0xFE)){
                                                pattern = 0x80;
                                                row=0xFE;
                                        }
                                        else{
                                                pattern <<= 1;
                                                row=(row>>1) |0x80;
                                        }

				}
				else if((x1<500)&&(x2<500)){
                                        if ((pattern == 0x80)&&(row != 0xEF)) {
                                                pattern = 0x80;
						row=(row<<1) |0x01;

                                        }
                                        else if((pattern != 0x80)&&(row == 0xEF)){
                                                row=0xEF;
						pattern <<= 1;
                                        }
                                        else if((pattern == 0x80)&&(row == 0xEF)){
                                                pattern = 0x80;
                                                row=0xEF;
                                        }
                                        else{
                                                pattern <<= 1;
                                                row=(row<<1) |0x01;
                                        }

				}
			}
			else if(t%sp2==0){
				if((x2>600)&&((x1>=500)&&(x1<=600))){
					if (row == 0xFE) {
						row = 0xFE;                                       
					}
					else {
						row=(row>>1) |0x80;
					}
				}
				else if((x2<500)&&((x1>=500)&&(x1<=600))){
					if(row==0xEF){
						row=0xEF;
					}
					else{
						row=(row<<1)|0x01;
					}
				}
			
			
				else if((x1>600)&&(x2>600)){
                                        if ((pattern == 0x01)&&(row != 0xFE)) {
                                                pattern = 0x01;
                                                row=(row>>1) |0x80;
                                        }
                                        else if((pattern != 0x01)&&(row == 0xFE)){
                                                row=0xFE;
                                                pattern >>= 1;
                                        }
                                        else if((pattern == 0x01)&&(row == 0xFE)){
                                                pattern = 0x01;
                                                row=0xFE;
                                        }
                                        else{
                                                pattern >>= 1;
                                                row=(row>>1) |0x80;
                                        }

                                }
                                else if((x1>600)&&(x2<500)){
                                        if ((pattern == 0x01)&&(row != 0xEF)) {
                                                pattern = 0x01;
                                                row=(row<<1) |0x01;
                                        }
                                        else if((pattern != 0x01)&&(row == 0xEF)){
                                                row=0xEF;
                                                pattern >>= 1;
                                        }
                                        else if((pattern == 0x01)&&(row == 0xEF)){
                                                pattern = 0x01;
                                                row=0xEF;
                                        }
                                        else{
                                                pattern >>= 1;
                                                row=(row<<1) |0x01;
                                        }
                                }
				else if((x1<500)&&(x2>600)){
                                        if ((pattern == 0x80)&&(row != 0xFE)) {
                                                pattern = 0x80;
                                                row=(row>>1) |0x80;
                                        }
                                        else if((pattern != 0x80)&&(row == 0xFE)){
                                                row=0xFE;
                                                pattern <<= 1;
                                        }
                                        else if((pattern == 0x80)&&(row == 0xFE)){
                                                pattern = 0x80;
                                                row=0xFE;
                                        }
                                        else{
                                                pattern <<= 1;
                                                row=(row>>1) |0x80;
                                        }

                                }
                                else if((x1<500)&&(x2<500)){
                                        if ((pattern == 0x80)&&(row != 0xEF)) {
                                                pattern = 0x80;
                                                row=(row<<1) |0x01;

                                        }
                                        else if((pattern != 0x80)&&(row == 0xEF)){
                                                row=0xEF;
                                                pattern <<= 1;
                                        }
                                        else if((pattern == 0x80)&&(row == 0xEF)){
                                                pattern = 0x80;
                                                row=0xEF;
                                        }
                                        else{
                                                pattern <<= 1;
                                                row=(row<<1) |0x01;
                                        }

                                }
			
			}
			t++;
			break;
		default:
			break;
	}
	PORTC = pattern;	
	PORTD = row;
	return state2;
}




int main(void) {

	DDRA=0x00;PORTA=0xFF;
	DDRC=0xFF;PORTB=0x00;
	DDRD=0xFF;PORTC=0x00;
	ADC_init();
	unsigned char i=0;
	tasks[i].state=adc1;
	tasks[i].period=1;
	tasks[i].elapsedTime=0;
	tasks[i].TickFct=&Shift_Tick;
	i++;
/*	tasks[i].state=speed;
        tasks[i].period=1;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Speed_Tick;
	i++;
	tasks[i].state=start2;
        tasks[i].period=50;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&LED2_Tick;
	i++;*/
	tasks[i].state=start;
        tasks[i].period=50;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&LED_Tick;
	TimerSet(1);
	TimerOn();
	while (1) {
	}
	return 1;
}
