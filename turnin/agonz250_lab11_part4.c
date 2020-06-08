/*	Author: agonz250
 *  Partner(s) Name: 
 *	Lab Section: 028 
 *	Assignment: Lab #11  Exercise #4
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
unsigned char tmpOut = 0x00; //output for keypad 
unsigned char x = 0;

unsigned char outputChar;


unsigned char cursor = 0; //index of cursor
//end of shared variables 

enum keypad_States {keypad_keypad} ;
int keypadSMTick(int state) {    	
	x = GetKeypadKey(); 

	switch(state) {
		case keypad_keypad:
			state = keypad_keypad;

			switch (x) {
				case '\0': tmpOut = 0x1F; break;
				case '1': tmpOut = 0x01; 
					break; 
				case '2': tmpOut = 0x02; 
					  break;

				case '3': tmpOut = 0x03; 
					break;

				case '4': tmpOut = 0x04; 
					  break;


				case '5': tmpOut = 0x05; 
					 break;


				case '6': tmpOut = 0x06; 	
					  break;

				case '7': tmpOut = 0x07; 
					  break;

				case '8': tmpOut = 0x08; 	
					  break;


				case '9': tmpOut = 0x09; 	
					  break;

				case 'A': tmpOut = 0x11; 	
					  break;
				
				case 'B': tmpOut = 0x12; 	
					  break;


				case 'C': tmpOut = 0x13; 
					  break;


				case 'D': tmpOut = 0x14;
					  break;


				case '*': tmpOut = 0x0E; 	
					  break;


				case '0': tmpOut = 0x00; 	
					  break;


				case '#': tmpOut = 0x0F;  
					  break;

				default: tmpOut = 0x1B; 
					 break; //should never occur, middle led
		
				}

		default: 
			state = keypad_keypad;
			break;
		}

       		return state; 	
}



//display 
enum display_States { display_display }; 
int displaySMTick(int state) {
	//unsigned char output;

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
			if ((tmpOut & 0xFF) != 0x1F) {
				
				if (cursor > 16) { //resets cursors if greater than 16
					cursor = 1;
				}
					
				LCD_Cursor(cursor);

				//If '#'
				if ((tmpOut & 0xFF) == 0x0E) {
					LCD_WriteData(tmpOut + 0x1C);
				}
				//If '*'
				else if ((tmpOut & 0xFF) == 0x0F) {
					LCD_WriteData(tmpOut + 0x14);
				}
				else {
					LCD_WriteData(tmpOut + '0'); //display
				}
				cursor++;
			}
			break;
	}
	//PORTB = output; 
	return state; 
}


int main(void) {
    /* Insert DDR and PORT initializations */
	//DDRB = 0xFF; PORTB = 0x00; //outputs, set to 0
	
	DDRC = 0xF0; PORTC = 0x0F; //PC7-4 outputs, PC3 - 0 inputs
	DDRA = 0xFF; PORTA = 0x00; //LCD data lines (dif from lab 7)
	DDRD = 0xFF; PORTD = 0x00; //LCD control lines 


	//declar array of tasks 
	static task task1, task2; 
	task *tasks[] = { &task1, &task2 };
        const unsigned short numTasks = sizeof(tasks)/sizeof(task*);	

	const char start = -1;

	//start the LCD 
	LCD_init(); 
	LCD_ClearScreen();
	LCD_DisplayString(1, "Congratulations!");

	//task 1 (keypad input)
	task1.state = start; //inital state task 
	task1.period = 100;	//task period 
	task1.elapsedTime = task1.period; //task current elapsed time 
	task1.TickFct = &keypadSMTick; //Function pointer

	//task 2 (displaySM)
	
	task2.state = start; //inital state task 
	task2.period = 200;	//task period 
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
