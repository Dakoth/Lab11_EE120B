/*	Author: agonz250
 *  Partner(s) Name: 
 *	Lab Section: 028 
 *	Assignment: Lab #11  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include "keypad.h"
#include "bit.h"
#include "scheduler.h"
#include "timer.h" 
#include "io.h"

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

//Shared variables 


//end of shared variables





//display 
enum display_States { display_display }; 
int displaySMTick(int state) {
	unsigned char output;

	switch (state) { //transitions 
		case display_display: 
			state = display_display;
			break;

		default: 
			state = display_display;
			break; 

	}

	switch (state) { //actions 
		case display_display: 
			//output = tmpB
			break;
	}
	//PORTB = output; 
	return state; 
}


int main(void) {
    /* Insert DDR and PORT initializations */
	//DDRB = 0xFF; PORTB = 0x00; //outputs, set to 0
	//DDRC = 0xF0; PORTC = 0x0F; //PC7-4 outputs, PC3 - 0 inputs
	
	DDRA = 0xFF; PORTA = 0x00; //LCD data lines (dif from lab 7)
	DDRD = 0xFF; PORTD = 0x00; //LCD control lines 


	//declar array of tasks 
	static task task1, task2; 
	task *tasks[] = { &task1, &task2 };
        const unsigned short numTasks = sizeof(tasks)/sizeof(task*);	

	const char start = -1;

	//start the LCD 
	LCD_init(); 

	//task 1 (message)
	task1.state = start; //inital state task 
	task1.period = 200;	//task period 
	task1.elapsedTime = task1.period; //task current elapsed time 
	task1.TickFct = &messageSMTick; //Function pointer

	//task 2 (displaySM)
	
	task2.state = start; //inital state task 
	task2.period = 10;	//task period 
	task2.elapsedTime = task2.period; //task current elapsed time 
	task2.TickFct = &displaySMTick; //Function pointer
	


	//GCD VVVV
	unsigned short i; 
	unsigned long GCD = tasks[0]->period;
	for (i = 1; i < numTasks; i++) {
		GCD = findGCD(GCD, tasks[i]->period);
	}

	TimerSet(GCD); 
	TimerOn();

	//unsigned short i; 
	while(1) {
		for (i = 0; i < numTasks; i++) {
			if ( tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += GCD; 
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
   }
