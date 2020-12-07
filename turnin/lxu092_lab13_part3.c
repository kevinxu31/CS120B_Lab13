/*	Author: lab
 *  Partner(s) Name: Luofeng Xu
 *	Lab Section:
 *	Assignment: Lab 13  Exercise 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: Youtube URL>https://youtu.be/2Ig4YO8zAuE
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef struct task{
        int state;
        unsigned long period;
        unsigned long elapsedTime;
        int(*TickFct)(int);
}task;

task tasks[3];
const unsigned short tasksNum=3;
const unsigned long tasksPeriod=50;
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
unsigned short x;

enum Shift_States{adc};
int Shift_Tick(int state){
	switch(state){
		case adc:
			break;
		default:
			state=adc;
			break;
	}
	switch(state){
		case adc:
			x=ADC;
			break;
		default:
			break;
	}
	return state;
}
unsigned char sp=20;
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
			if (((x>600)&&(x<=700))||((x<500)&&(x>=400))){
				sp=20;
			}
			else if(((x>700)&&(x<=800))||((x<400)&&(x>=300))){
				sp=10;
			}
			else if(((x>800)&&(x<=900))||((x<300)&&(x>=200))){
                                sp=5;
                        }
			else if((x>900)||(x<200)){
                                sp=2;
                        }
			break;
		default:
			break;
	}
	return state1;

}


enum LED_States {start,init,left,right};
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
			if((x>=500)&&(x<=600)){
				state2=init;				
			}
			else if(x<500){
				t=0;
				state2=left;
			}
			else if (x>600){
				t=0;
				state2=right;
			}
			break;
		case left:
			if(x<500){
				state2=left;
			}
			else{
				state2=init;
			}
			break;
		case right:
			if(x>600){
				state2=right;
			}
			else{
				state2=init;
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
		case right:
			if(t%sp==0){
				if (pattern == 0x01) { 
					pattern = 0x80;		  			
				}
				else { 
					pattern >>= 1;
				}
			}
			t++;
			break;
		case left:
			if(t%sp==0){
				if(pattern==0x80){
					pattern=0x01;
				}
				else{
					pattern<<=1;
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
	tasks[i].state=adc;
	tasks[i].period=50;
	tasks[i].elapsedTime=0;
	tasks[i].TickFct=&Shift_Tick;
	i++;
	tasks[i].state=speed;
        tasks[i].period=50;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Speed_Tick;
	i++;
	tasks[i].state=start;
        tasks[i].period=50;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&LED_Tick;
	TimerSet(50);
	TimerOn();
	while (1) {
	}
	return 1;
}
