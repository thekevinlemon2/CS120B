/*
 * Final Project.c
 *
 * Created: 8/24/2017 12:51:43 PM
 * Author : Kevin TK Nguyen
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
unsigned char bl_num = 0x00;
unsigned char tl_num = 0x00;
unsigned char songdone = 0;
unsigned char music_period = 200;
const char *catalog[] = {"1) Sad Machine:(", "2) Basic", "3) Shootin Stars", "4) Scale", "5) Mary's Lambo"};
const char *notes[5] = {
"eeexeeexeeexxxxxbbGGbbbbGGFFeeexeeexeeexxxxxbbGGbbbbGGFFeeexeeexeeexxxxxbbGGbbbbGGFFeeexeeexeeexxxxxbbccGGGGGGFFeeeeeeeeeeee\0", //shad machine :(
"XXXXaaaaXXXXaaaaccccXXXXccccFFFFXXXXFFFFaaaaXXXXaaaaddddXXXXddddFFFFXXXXFFFFeeeeXXXXeeeeGGGGXXXXGGGG\0", //that one song
"eeeeeeeexxeexxEEEEXXXX1144eeeeeeeexxeexxEEEEXXXX1144eeeeeeeexxeexxEEEEXXXX1144eeeeeeeexxeexxEEEEXXXX\0", //shooting stars
"7XXX6XXX5XXX4XXX3XXX2XXX1XXXCXXXdXXXDXeXEXFXgXGXaXAXbXBXcXhXHXiXIIIIIIIIIIII\0", //scale
"EEEEDDDDCCCCDDDDEEEEEEEEEEEEXXXXDDDDDDDDDDDDXXXXEEEEEEEEEEEEXXXXEEEEDDDDCCCCDDDDEEEEEEEEEEEEEEEEDDDDDDDDEEEEDDDDCCCCCCCCCCCCCC\0\0",
};
const char *catalog_nn[] = {"Playing         Sad Machine :(", "Playing         Basic", "Playing         Shootin Stars", "Playing         Scale", "Playing         Mary's Lambo"};
unsigned char write2display = 0;
unsigned char songnum = 0;
unsigned char increment = 1; //flag for incrementing song selections
unsigned char play_song = 0; //flag for playing song
unsigned char note_number = 0;


void TimerISR() {
	TimerFlag = 1;
}

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

double n2f(unsigned char note) //function for converting letter note to numerical frequency
{
	if (note == '7') //F3
	{
		return 174.61;
	}
	else if (note == '6') //G3b
	{
		return 185.00;
	}
	else if (note == '5') //G
	{
		return 196.00;
	}
	else if (note == '4') //A3b
	{
		return 207.65;
	}
	else if (note == '3') //A3
	{
		return 220.00;
	}
	else if (note == '2') //B3b
	{
		return 233.08;
	}
	else if (note == '1') //B3
	{
		return 246.94;
	}
	else if (note == 'C') //C4
	{
		return 261.63;
	}
	else if (note == 'd')
	{
		return 277.2;
	}
	else if (note == 'D')
	{
		return 293.66;
	}
	else if (note == 'e')
	{
		return 311.1;
	}
	else if (note == 'E')
	{
		return 329.63;
	}
	else if (note == 'F')
	{
		return 349.23;
	}
	else if (note == 'g')
	{
		return 370;
	}
	else if (note == 'G')
	{
		return 392;
	}
	else if (note == 'a')
	{
		return 415.3;
	}
	else if (note == 'A')
	{
		return 440;
	}
	else if (note == 'b')
	{
		return 466.2;
	}
	else if (note == 'B')
	{
		return 493.88;
	}
	else if (note == 'c') //higher C (C5)
	{
		return 523.25;
	}
	else if (note == 'h') //C# or Db
	{
		return 554.37;
	}
	else if (note == 'H') //D5
	{
		return 587.33;
	}
	else if (note == 'i') //D# or Eb
	{
		return 622.25;
	}
	else if (note == 'I') //E5
	{
		return 659.25;
	}
	else if (note == 'J') //F5
	{
		return 698.46;
	}
	else if (note == 'X')
	{
		return 0;
	}
}

void n2LED(unsigned char note) //function for converting letter note to numerical frequency
{
	PORTA = PORTA & 0x1F;
	PORTB = PORTB & 0xE0;
	PORTD = PORTD & 0xC0; //this chunk resets all LEDS to be off first
	if (note == '7' || note == '6') //F3
	{
		PORTA = PORTA | 0x40;
		return;
	}
	else if (note == '5' || note =='4') //G
	{
		PORTD = PORTD | 0x20;
		return;
	}

	else if (note == '3' || note =='2') //A3
	{
		PORTD = PORTD | 0x10;
		return;
	}
	else if (note == '1' || note =='C') //B3
	{
		PORTD = PORTD | 0x08;
		return;
	}
	else if (note == 'd' || note =='D')
	{
		PORTD = PORTD | 0x04;
		return;
	}
	else if (note == 'e' || note =='E')
	{
		PORTD = PORTD | 0x02;
		return;
	}
	else if (note == 'F' || note =='g')
	{
		PORTD = PORTD | 0x01;
		return;
	}
	else if (note == 'G' || note =='a')
	{
		PORTB = PORTB | 0x10;
		return;
	}
	else if (note == 'A' || note =='b')
	{
		PORTB = PORTB | 0x08;
		return;
	}
	else if (note == 'B' || note =='c')
	{
		PORTB = PORTB | 0x04;
		return;
	}
	else if (note == 'h' || note =='H') //C# or Db
	{
		PORTB = PORTB | 0x02;
		return;
	}
	else if (note == 'i' || note =='I') //D# or Eb
	{
		PORTB = PORTB | 0x01;
		return;
	}
	else if (note == 'J') //F5
	{
		PORTA = PORTA | 0x20;
		return;
	}
	else if (note == 'X')
	{
		return;
	}
}


enum LCD_states {init, scroll_p, scroll_r, playing} LCD_state; //init tells user to use buttons to select song, scroll goes through songs, and playing is when song is playing duh
void tck_LCD(unsigned char button)
{
	switch(LCD_state) //transitions
	{
		case init:
			if (button == 0x08 || button == 0x04) //if user presses up or down button start displaying song names
			{
				LCD_state = scroll_p;
				break;
			}
			else
			{
				LCD_state = init; //if user doesn't press arrow buttons then LCD stays on start screen 
				break;
			}
		case scroll_p:
			if (button == 0x00)
			{
				LCD_state = scroll_r;
				break;
			}
		case scroll_r:
			if (button == 0x02) //user pressed play
			{
				write2display = 1;
				LCD_state = playing;
				break;
			}
			else if (button == 0x08 || button == 0x04) //if user presses up or down go back to scroll_p
			{
				LCD_state = scroll_p;
				break;
			}
			else
			{
				LCD_state = scroll_r;
				break;
			}
		case playing:
			if (button == 0x01 || play_song == 0) //user pressed stop so go back to scroll
			{
				LCD_state = scroll_p;
				break;
			}
			else
			{
				LCD_state = playing;
				break;
			}
		default:
			LCD_state = init;
			break;
	}
	switch(LCD_state) //actions
	{
		case init:
			LCD_DisplayString(1, "Use arrows to   pick a song :)");
			songnum = 0;
			break;
		case scroll_p:
			play_song = 0;
			if (button == 0x08)// if they pressed up
			{
				if (songnum == 0)
				{
					LCD_DisplayString(1, catalog[0]);
				}
				else
				{
					if (increment == 1)
					{
						songnum--;
						increment = 0;
					}
				}
			}
			else if (button == 0x04)// if they pressed down
			{
				if (songnum == 4)
				{
					LCD_DisplayString(1, catalog[4]);
				}
				else
				{
					if (increment == 1)
					{
						songnum++;
						increment = 0;
					}
				}
			}
			LCD_DisplayString(1, catalog[songnum]);
			break;
		case scroll_r:
			increment = 1;
			break;
		case playing:
			play_song = 1;
			if (write2display == 1)
			{
				LCD_DisplayString(1, catalog_nn[songnum]);
				//LCD_WriteData(catalog_nn[songnum]);
				write2display = 0;
			}
			break;
	}
}

enum speaker_states {stop, play,} speaker_state;
void tck_jukebox(const char *songnotes[])
{
	switch(speaker_state) //transitions
	{
		case stop:
			if (play_song)
			{
				speaker_state = play;
				break;
			}
			else
			{
				speaker_state = stop;
				break;
			}
		case play:
			if(play_song == 0)
			//if (songdone)
			{
				speaker_state = stop;
			}
			else
			{
				speaker_state = play;
			}
	}
	
	switch(speaker_state) //actions
	{
		case stop:
			set_PWM(0);
			note_number = 0;
			break;
		
		case play:
			if (songnotes[note_number] == '\0')
			{
				set_PWM(0);
				play_song = 0;
			}
			else
			{
				set_PWM(n2f(songnotes[note_number]));
				note_number++;
			}

			break;
	}
}


enum LED {not_lit, lit} LEDS;	
void tck_LEDS(const char *songnotes[])
{
	switch(LEDS) //transitions
	{
		case not_lit:
			if (play_song) {
				LEDS = lit;
			}
			else {
				LEDS = not_lit;
				break;
			}
		case lit:
			if (play_song) {
				LEDS = lit;
				break;
			}
			else {
				LEDS = not_lit;
				break;
			}
		default: 
			LEDS = not_lit;
			break;
	}
	
	switch(LEDS) //actions
	{
		case lit: //all LEDs on
		/*
			PORTA = PORTA | 0xF0;
			PORTB = PORTB | 0x1F;
			PORTD = PORTD | 0x3F;
		*/
			n2LED(songnotes[note_number]);
			break;
		
		case not_lit: //all LEDS off
			PORTA = PORTA & 0x1F;
			PORTB = PORTB & 0xE0;
			PORTD = PORTD & 0xC0;
			break;
	}
}

enum toggles {off, on} toggle;
void tck_toggle(unsigned char tempaa)
{
	switch(toggle)
	{	
		case off:
			if (tempaa)
			{
				toggle = on;
				break;
			}
			else {
				toggle = off;
				break;
			}
		case on:
			if (tempaa)
			{
				toggle = on;
				break;
			}
			else {
				toggle = off;
				break;
			}
	}
	switch(toggle)
	{	
		case off:
			music_period = 200;
			break;
		case on:
			music_period = 100;
			break;
	}
}

//intial test strings for songs

/*
char mary_lamb[32] = {'E', 'D', 'C', 'D', 'E', 'E', 'E', 'X', 'D', 'D', 'D', 'X', 'E', 'E', 'E', 'X', 'E', 'D', 'C', 'D', 'E', 'E', 'E', 'E', 'D', 'D', 'E', 'D', 'C', 'C', 'C', 'X'};
char lambtest[] = "EEEEDDDDCCCCDDDDEEEEEEEEEEEEXXXXDDDDDDDDDDDDXXXXEEEEEEEEEEEEXXXXEEEEDDDDCCCCDDDDEEEEEEEEEEEEEEEEDDDDDDDDEEEEDDDDCCCCCCCCCCCCX";
char sad_machine[] = "eeeeXbGbbGFeeeX";
char sad_machine2[] = "exexexxxbGbbGF";
char sad_machine3[] = "eeexeeexeeexxxxxbbGGbbbbGGFFeeexeeexeeexxxxxbbGGbbbbGGFFeeexeeexeeexxxxxbbGGbbbbGGFFeeexeeexeeexxxxxbbccGGGGGGFFeeeeeeeeeeee";
char doot[] = "XXXXaXacXcFXFaXadXdFXFeXeGXG";
char SS[] = "eeeexexEEEX14eeeexexEEEX";
char scale_test[] = "7XXX6XXX5XXX4XXX3XXX2XXX1XXXCXXXdXXXDXeXEXFXgXGXaXAXbBchHiIIIIIIIIIIIIIIII";
unsigned char notey = 'C';
*/

unsigned char music_elapsed_time = 200;
unsigned char timerPeriod = 100;
//int i = 0;

int main(void)
{
	DDRA = 0xE0; PORTA = 0x1F;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	LCD_init();
	//LCD_DisplayString(1, catalog[0]);
	unsigned char tempa;
	unsigned char speed_toggle = 0x00;
	PWM_on();
	TimerSet(timerPeriod);
	TimerOn();
	LCD_state = init;
    /* Replace with your application code */
    while (1) 
    {
		/*
		PORTA = PORTA | 0xF0;
		PORTB = 0x1F;
		PORTD = PORTD | 0x7F;
		*/
		
		tempa = ~PINA & 0x0F;
		speed_toggle = ~PINA & 0x10;
		tck_LCD(tempa);
		tck_toggle(speed_toggle);
		if (music_elapsed_time >= music_period)
		{
			tck_jukebox(notes[songnum]);
			tck_LEDS(notes[songnum]);
			music_elapsed_time = 0;
		}
		while (!TimerFlag){}
		TimerFlag = 0;
		music_elapsed_time += timerPeriod;
		
    }
}

