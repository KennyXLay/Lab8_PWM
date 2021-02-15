/*	Author: lab
 *  Partner(s) Name: Kenny
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include <avr/interrupt.h>
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
unsigned char tmpB = 0x00;

void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}
void TimerOff(){
	TCCR1B = 0x00;
}
void TimerISR(){
	TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;	
}


void set_PWM(double frequency){
	static double current_frequency;
	if(frequency != current_frequency){
		
		if(!frequency){TCCR3B &= 0x08;}
		else{
			TCCR3B |= 0x03;}
		if(frequency < 0.954){
			OCR3A = 0xFFFF;
		}
		else if(frequency > 31250){
			OCR3A = 0x0000;
		}
		else{OCR3A = (short)(8000000/(128 * frequency)) - 1;
		}
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on(){
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off(){
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}


enum States{init, off, on, scaleUp, scaleDown} state;

double freq[8] = {261.63,293.66,329.63,349.23,392,440,493.88,523.25};
unsigned char i = 0;

void Tick(){
	unsigned char tmpA = ~PINA;
	switch(state){
		case init: 
			state = off; 
			break;
		case off: 
			if (tmpA == 0x04){
				PWM_on();
				state = on;	
			}
			else{state = off;}
			   break;
		case on:
			  if (tmpA == 0x01){state = scaleUp;}
			  else if (tmpA  == 0x02){state = scaleDown;}
			  else if (tmpA  == 0x04){state = off;}
			  else{state = on;}
			  break;
		case scaleUp: 
			  if(tmpA == 0x04){
				  state = off;
			  }
			  else{
				  state = on;
			  }
			  break;
		case scaleDown: 
			  if(tmpA == 0x04){
                                  state = off;
                          }
                          else{
                                  state = on;
                          }
    			  break;
		default: 
			  state = init; 
			  break;
	}

	switch(state){
		case off: 
			PWM_off(); 
			i = 0; 
			break;
		case on: 
			set_PWM(freq[i]); 
			break;
		case scaleUp: 
			if (i < 7){	
				i++;
			}	
			break;
		case scaleDown: 
			if (i > 0){
				i--;
			}	
			break;
		default: 
			break;

	}
}
int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
	PWM_on();
	TimerSet(100);
	TimerOn();
	
    while (1) {
	Tick();
	while(!TimerFlag);
	TimerFlag = 0;
    }
    PWM_off();
    return 1;
}
