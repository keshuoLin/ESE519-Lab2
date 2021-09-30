/*
 * PartE.c
 *
 * Created: 2021/9/28 15:02:09
 * Author : Annie
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <string.h>

#define F_CPU 16000000UL

#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

volatile int time_len=0;
volatile int edge=0;

int fall_or_rise=0;//A flag to check whether it is falling or rising, falling edge:1; rising edge:0.

char String[20];

char Morse_char[]="";//

int i=0;

	char* Morse_check[]={"HERE",".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....",
		"..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-",
		".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",".----","..--- ",
		"...--","....-",".....","-....","--...","---..",
	"----.","-----","END"};
	char decode[]={'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V',
					'W','X','Y','Z','1','2','3','4','5','6','7','8','9','0'};
	
void Initialize(){
	cli();					//Disable all global interrupts
	DDRB &=~(1<<DDB0);		//Set PB0 as an input pin
	DDRB |=(1<<DDB5);		//Set PB5 as an output pin
	DDRB |=(1<<DDB1);		//Set PB1 as an output bin
	DDRB |=(1<<DDB2);		//Set PB2 as an output bin	
	PORTB |= (1<<PORTB0);   //Enable pull up resistor on PB0
	
	DDRB |=(1<<DDB1);
	
	//Enable Timer1 and set it to be normal
	TCCR1A &=~(1<<WGM10);
	TCCR1A &=~(1<<WGM11);
	TCCR1B &=~(1<<WGM12);
	TCCR1B &=~(1<<WGM13);
	
	//Set Timer1 clock to be internal divided by 1024
	//After timer prescale f_clk = 16MHz/256 = 15625kHz
	//Period=1/15625Hz=0.064ms
	TCCR1B |=(1<<CS12);
	TCCR1B &=~(1<<CS11);
	TCCR1B |=(1<<CS10);
	
	//Write a logic one to ICF1 to clear it
	TIFR1 |= (1<<ICF1);
	
	//Looking for a falling edge
	TCCR1B &= ~(1<<ICES1);
	
	//Enable Timer/Counter1 input capture interrupt
	TIMSK1 |= (1<<ICIE1);

	sei();					//Enable global interrupts
}

ISR(TIMER1_CAPT_vect){

	
	time_len= ICR1-edge;
	edge=ICR1;
	
	UART_init(BAUD_PRESCALER);
	
	if (fall_or_rise)//falling edge? We are pressing a button.
	{
		/*Measure the time length
		30ms/0.064ms=469tics
		200ms/0.064ms=3125tics
		400sm/0.064ms=6250tics
		*/
		if ((time_len>=469)&&(time_len<3125))//Button being pressed from 30 to 200ms, it is a dot
		{	//sprintf(String,".");//A DOT "." is written into String
			//UART_putstring(String);
			PORTB|=(1<<PORTB1);
			Morse_char[i]='.';
		}
		if ((time_len>=3125)&&(time_len<6250))//Button being pressed from 200 to 400ms, it is a dash
		{	//sprintf(String,"-");
			//UART_putstring(String);
			PORTB|=(1<<PORTB2);
			Morse_char[i]='-';
		}
		i++;
		fall_or_rise=0;

		//sprintf(String,"\n");
		//UART_putstring(String);
		
	}
	
	else{
		if (time_len>6250)//Button being pressed longer than 400ms, it is a space
		{	
		
		//sprintf(String,"It is %s",Morse_char[10]);
		UART_putstring(Morse_char);
		sprintf(String,"\n");
		UART_putstring(String);		
		Decoder();
		i=0;
		
		
		
		memset(Morse_char, 0, 10);

		}
		fall_or_rise=1;
	}
	
	PORTB^=(1<<PORTB5);
	TCCR1B^=(1<<ICES1);

}

void Decoder()
{	//String Morse_str=new String(Morse_char);
	int j=0;
	UART_putstring(&Morse_check[j]);
	while (Morse_check[j]!="END")
	{
		if(Morse_check[j]==Morse_char)
		{
			//sprintf(String,"here");
			
			//UART_putstring(decode[j]);
			break;
		}
		j++;
	}
}

int main(void)
{
	Initialize();
	
	while(1)
	{	if (PORTB1)
		{
			_delay_ms(50);
			PORTB&=~(1<<PORTB1);
		}
		
		if (PORTB2)
		{	_delay_ms(50);
			PORTB&=~(1<<PORTB2);
		}
						
	}
}
