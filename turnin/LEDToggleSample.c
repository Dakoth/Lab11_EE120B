/*	Author: agonz250
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #
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

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

/*
unsigned char GetKeypadKey() { 
	PORTC = 0xEF;
	asm("nop");
	if (GetBit(PINC, 0) == 0) { return('1'); } 
	if (GetBit(PINC, 1) == 0) { return('4'); } 
	if (GetBit(PINC, 2) == 0) { return('7'); } 
	if (GetBit(PINC, 3) == 0) { return('*'); } 

	//check keys in col 2
	PORTC = 0xDF; 
	asm("nop");
	if (GetBit(PINC, 0) == 0) { return('2'); } 
	if (GetBit(PINC, 1) == 0) { return('5'); } 
	if (GetBit(PINC, 2) == 0) { return('8'); } 
	if (GetBit(PINC, 3) == 0) { return('0'); } 

	//check keys in col 3
	PORTC = 0xBF; 
	asm("nop");
	if (GetBit(PINC, 0) == 0) { return('3'); } 
	if (GetBit(PINC, 1) == 0) { return('6'); } 
	if (GetBit(PINC, 2) == 0) { return('9'); } 
	if (GetBit(PINC, 3) == 0) { return('#'); } 


	//checks keys in col 4
	PORTC = 0x7F; 
	asm("nop");
	if (GetBit(PINC, 0) == 0) { return('A'); } 
	if (GetBit(PINC, 1) == 0) { return('B'); } 
	if (GetBit(PINC, 2) == 0) { return('C'); } 
	if (GetBit(PINC, 3) == 0) { return('D'); } 



	return ('\0'); //default value 
}
*/		

//Shared variables 
unsigned char led0_output = 0x00;
unsigned char led1_output = 0x00;
unsigned char pause = 0; 
//end of shared variables 


//monitors button to PA0 
//when button pressed, "pause" is toggled 
enum pauseButtonSM_States { pauseButton_wait, pauseButton_press, pauseButton_release }; 
int pauseButtonSMTick(int state) {
	//Local varaibles 
	unsigned char press = ~PINA & 0x01;

	switch (state) { 
		case pauseButton_wait:
			state = press == 0x01? pauseButton_press: pauseButton_wait; 
			break; 

		case pauseButton_press: 
			state = pauseButton_release;
			break; 

		case pauseButton_release:
		       state = press == 0x00? pauseButton_wait: pauseButton_press;
	       		break; 

		default: 
			state = pauseButton_wait;
			break;		
	}

	switch(state) {	//actions 
		case pauseButton_wait: break;
		case pauseButton_press:
			pause = (pause == 0) ? 1 : 0;
			break;

		case pauseButton_release: 
			break; 
	}
	return state; 
}


//toggleLED0
//if paused: Don't toggle LED connected to PB0
//if unpaused: toggle LED connected to PB0
enum toggleLED0_States { toggleLED0_wait, toggleLED0_blink} ;

int toggleLED0SMTick ( int state) {
	switch (state) {
		case toggleLED0_wait: 
			state = !pause? toggleLED0_blink: toggleLED0_wait; 
			break; 
	
		case toggleLED0_blink:
			state = pause? toggleLED0_wait: toggleLED0_blink; 
		        break;

		default: 
	       		state = toggleLED0_wait;
			break;		
	}

	switch (state) { 
		case toggleLED0_wait: break;
		case toggleLED0_blink:
			led0_output = (led0_output == 0x00) ? 0x01 : 0x00;
			break;
	}

	return state; 
}


//toggleLED1
//paused: don't toggle PB1
//unpaused: toggle PB1
enum toggleLED1_States { toggleLED1_wait, toggleLED1_blink} ;

int toggleLED1SMTick ( int state) {
	switch (state) {
		case toggleLED1_wait: 
			state = !pause? toggleLED1_blink: toggleLED1_wait; 
			break; 
	
		case toggleLED1_blink:
			state = pause? toggleLED1_wait: toggleLED1_blink; 
		        break;

		default: 
	       		state = toggleLED1_wait;
			break;		
	}

	switch (state) { 
		case toggleLED1_wait: break;
		case toggleLED1_blink:
			led1_output = (led1_output == 0x00) ? 0x01 : 0x00;
			break;
	}

	return state; 
}

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
			output = led0_output | led1_output << 1; 
			break;
	}
	PORTB = output; 
	return state; 
}


int main(void) {
    /* Insert DDR and PORT initializations */
	//DDRB = 0xFF; PORTB = 0x00; //outputs, set to 0
	//DDRC = 0xF0; PORTC = 0x0F; //PC7-4 outputs, PC3 - 0 inputs
	//unsigned char x; 

	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;

	//declar array of tasks 
	static task task1, task2, task3, task4; 
	task *tasks[] = { &task1, &task2, &task3, &task4 };
        const unsigned short numTasks = sizeof(tasks)/sizeof(task*);	

	const char start = -1;

	//task 1 (pauseBUttonToggleSM)
	task1.state = start; //inital state task 
	task1.period = 50;	//task period 
	task1.elapsedTime = task1.period; //task current elapsed time 
	task1.TickFct = &pauseButtonSMTick; //Function pointer

	//task 2 (toggleLED0SM)
	task2.state = start; //inital state task 
	task2.period = 500;	//task period 
	task2.elapsedTime = task2.period; //task current elapsed time 
	task2.TickFct = &toggleLED0SMTick; //Function pointer

	//task 3 (toggleLED1SM)
	task3.state = start; //inital state task 
	task3.period = 1000;	//task period 
	task3.elapsedTime = task3.period; //task current elapsed time 
	task3.TickFct = &toggleLED1SMTick; //Function pointer

	//task 4 (displaySM)
	task4.state = start; //inital state task 
	task4.period = 10;	//task period 
	task4.elapsedTime = task4.period; //task current elapsed time 
	task4.TickFct = &displaySMTick; //Function pointer

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
 /*
    while (1) {
	    
    	
	x = GetKeypadKey(); 
	switch (x) {
		case '\0': PORTB = 0x1F; break;
		case '1': PORTB = 0x01; break; 
		case '2': PORTB = 0x02; break;
		case '3': PORTB = 0x03; break; 
		case '4': PORTB = 0x04; break;
		case '5': PORTB = 0x05; break; 
		case '6': PORTB = 0x06; break;
		case '7': PORTB = 0x07; break; 
		case '8': PORTB = 0x08; break;
		case '9': PORTB = 0x09; break; 
		case 'A': PORTB = 0x0A; break;
		case 'B': PORTB = 0x0B; break; 
		case 'C': PORTB = 0x0C; break;
		case 'D': PORTB = 0x0D; break; 
		case '*': PORTB = 0x0E; break;
		case '0': PORTB = 0x00; break; 
		case '#': PORTB = 0x0F; break;
		default: PORTB = 0x1B; break; //should never occur, middle led
		
	}*/
   // }
    //return 1;
//}
