/*	Author: lab
 *  Partner(s) Name: Luofeng Xu
 *	Lab Section:
 *	Assignment: Lab 13  Exercise 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: Youtube URL>https://youtu.be/GfrH1zJgq6E
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void ADC_init(){
	ADCSRA |=(1<<ADEN)|(1<<ADSC)|(1<<ADATE);
}

unsigned char low;
unsigned char high;
int main(void) {

	DDRB=0xFF;PORTB=0x00;
	DDRD=0xFF;PORTD=0x00;
	ADC_init();
	while (1) {

		unsigned short value=ADC;
		low=(char)value;
		high=(char)(value>>8);
		PORTB=low;
		PORTD=high;
	}

	return 1;
}
