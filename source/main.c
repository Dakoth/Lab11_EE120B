/*	Author: agonz250
 *  Partner(s) Name: 
 *	Lab Section: 028 
 *	Assignment: Lab #11  Exercise #1
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
//unsigned char tmpB = 0x00; 
//unsigned char x = 0;

unsigned char msg[] = "CS120B is Legend... wait for it DARY!"; //msg
unsigned char tmp[50] = "";

unsigned char i = 0;
//end of shared variables 


enum message { mess_message };
int messageSMTick(int state) {
	switch(state) { //transitions 
		case mess_message:
			state = mess_message;

			if(msg[i] != '\0') { //If haven't reached null char.
				LCD_Cursor(14);
				LCD_writeData(msg[i]);
			}	
			else {
				i = 0;
			}
			break;

		default: 
			state = mess_message;
			break;
	}

	switch (state) { //actions
		case mess_message:
			i++;
			break;
	}
	return state; 
}


/*
enum keypad_States {keypad_keypad} ;
int keypadSMTick(int state) {    	
	x = GetKeypadKey(); 

	switch(state) {
		case keypad_keypad:
			state = keypad_keypad;

			switch (x) {
				case '\0': tmpB = 0x1F; break;
				case '1': tmpB = 0x01; break; 
				case '2': tmpB = 0x02; break;
				case '3': tmpB = 0x03; break; 
				case '4': tmpB = 0x04; break;
				case '5': tmpB = 0x05; break; 
				case '6': tmpB = 0x06; break;
				case '7': tmpB = 0x07; break; 
				case '8': tmpB = 0x08; break;
				case '9': tmpB = 0x09; break; 
				case 'A': tmpB = 0x0A; break;
				case 'B': tmpB = 0x0B; break; 
				case 'C': tmpB = 0x0C; break;
				case 'D': tmpB = 0x0D; break; 
				case '*': tmpB = 0x0E; break;
				case '0': tmpB = 0x00; break; 
				case '#': tmpB = 0x0F; break;
				default: tmpB = 0x1B; break; //should never occur, middle led
		
				}
			break;

		default: 
			state = keypad_keypad;
			break;
		}
		//switch (state) PROBABLY NONE NEEDED
		//

       		return state; 	
}
*/

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
			//output = tmpB; 
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
	task1.period = 100;	//task period 
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
