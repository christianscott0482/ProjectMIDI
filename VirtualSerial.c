/*
             LUFA Library
     Copyright (C) Dean Camera, 2015.


  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/


/*
  Copyright 2015  Dean Camera (dean [at] fourwalledcubicle [dot] com)


  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.


  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/


/* This code is based on Dean Camera's "VirtualSerial.c" from the Demo code included 
 * with the LUFA library.  Unnecessary parts have been removed and the "High Low"     
 * game has been dropped in largely unmodified.  This is meant to demonstrate a         
 * minimal USB device on an AVR.  From the AVR perspective the USB is simply          
 * stdin and stdout (with a CONNECTED flag).  From the PC's perspective the AVR is   
 * a serial port.  It is a greatly simplified serial port in the sense that baud rate and        
 * several other settings do not matter.  (The AVR can access these settings, but this  
 * is not included in this code.)
 */



#include "VirtualSerial.h"
#include "stdlib.h"
#include "stdio.h"
#include "util/delay.h"


extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;  
// At the bottom of this file
// It is essential, but the documentation didn't give me the information I needed to create one
// So I used the one from the example, but I don't feel good about it.

// USB defines
#define CONNECTED (VirtualSerial_CDC_Interface.State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR)

// Timer macros/defines
// 	PWM period in CTC mode = 2 * (Overflow Value + 1) * System Clock Period
//	Desired PWM period = 1/freq
//	1/FREQ = 2 * (OCR + 1) * CLK_PER + 1
//	OCR + 1 = 1/(FREQ * 2 * CLK_PER)
//	OCR = CLK_FREQ/(2 * FREQ) - 1
#define CLK_FREQ 8000000
#define FREQ_TO_OCR(X) (CLK_FREQ / (2 * X)) - 1
#define SET_TOP1_H(X) OCR1AH = FREQ_TO_OCR((X & 0xFF00) >> 8)
#define SET_TOP1_L(X) OCR1AL = FREQ_TO_OCR(X & 0xFF)
#define RESET1 TCNT1 = 0
#define CHECK_OVERFLOW (TIFR0 &  (1 << TOV0))
#define CLEAR_OVERFLOW TIFR0 |= (1 << TOV0)
#define SET_TOP0(X) OCR0A = X  
#define RESET0 TCNT0 = 0
#define CHECK0 (TIFR0 & (1 << OCF0A))
#define CLEAR0 TIFR0 |= (1 << OCF0A)

static FILE USBSerialStream;  // This will become stdin and stdout


void play_note(int freq, int bpm, char frac);     // Plays a single note
void play_song(void);				  // Play song from serial
void bruces_usb_init(void);	       		  // USB initialization
void timer_init(void);            		  // Initialize timer for use with buzzer
void delay(int bpm, int frac);		          // Delay using timer 0
int scan_int(void);				  // Scan integer value from serial

/* Below is a minimal main function for doing USB communication through a     
 * Virtual serial port.  the function bruces_usb_init does the minimal initialization
 * then the program waits for a connection (PC opens port) before calling a       
 * function to do whatever needs to be done.  In this case it is a high-low           
 * guessing game from the 477 web site.  It **IS** important, however that        
 * whatever program is run, it must spend the majority of its time blocked         
 * waiting for I/O (e.g., in a call to scanf)
 */ 
                             
int main(void)
{
	//char c;
	// Initialization
        bruces_usb_init();  
	timer_init();
	
	while(1)
        {
               	while(!CONNECTED) // Wait for connection
               	{
       	  	     	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        		USB_USBTask();
                }
               	play_song();
        } 
}


/* This code is essentially identical to the high_low_server code that  */
/* is one of the examples on the 477 page.  File I/O, stdin/stdout and  */
/* serial I/O are all incredibly similar and almost interchangeable           */


void play_song()
{
	int i = 0;			// Counter
	int freq = 0;			// Note frequency
	int frac = 0;			// Note fraction
	int bpm = 0;			// BPM
	int note_num = 0;		// Number of notes
	
	bpm = scan_int();
	printf("int%d\n\r", bpm);

	note_num = scan_int();
	printf("intnote_num%d\r\n", note_num);

	for (i = 0; i < note_num; i++) {
		freq = scan_int();
		printf("frequency:%d\r\n", freq);

		frac = scan_int();
		printf("frac:%d\r\n", frac);

		play_note(freq, bpm, frac);		
	}

	return;   
}

void play_note(int freq, int bpm, char frac) {
	
	// PWM in Phase & Frequency Correct PWM mode
	//	f = f_clk / (2 * N * ICRF1)
	// 	f = 8000000 / (2 * 8 * ICRF1)
	//	ICRF1 = 8000000 / (16 * f)	
	
	int icr = 8000000 / (16 * freq);
	ICR1H = (icr & 0xFF00) >> 8;
	ICR1L = (icr & 0xFF);
	OCR1AH = ((icr/2) & 0xFF00) >> 8;
	OCR1AL = ((icr/2) & 0xFF);

	delay(bpm, frac);
	return;
}

int scan_int(void)
{
	char buf[10];
	scanf("%s", buf);
	printf("string%s\n\r", buf);
	return atoi(buf);
}

/* Minimal configuration for USB I/O through stdin and stdout.  Disable watchdog timer,  */
/* call USB_init in LUFA library, creating a blocking stream (note it is important to be        */
/* blocked most of the time for the USB stuff to work correctly), and set stdin and stdout */
/* to point to it. Finally,  enable interrupts                                                                             */


void bruces_usb_init(void)
{                   
        wdt_disable();     //Make sure the Watchdog doesn't reset us
        USB_Init();  // Lufa library call to initialize USB
        /* Create a stream for the interface */
        CDC_Device_CreateBlockingStream(&VirtualSerial_CDC_Interface,&USBSerialStream);
        stdin=&USBSerialStream;  //By setting stdin and stdout to point to the stream we
        stdout=&USBSerialStream; //can use regular printf and scanf calls
        GlobalInterruptEnable();      // interrupts need to be enabled
 }

void delay(int bpm, int frac)
{
	int overflow_req = 0;	// Required number of overflows for delay
	long count_req = 0;	// Required number of timer counts
	int remainder = 0;	// Total number of remaining timer counts for delay
	int overflow_cnt = 0;	// Number of overflows elapsed	

	// Timer0 counting frequency is 31.25 kHz
	// 	counting period is 1/31.25kHz = 32us
	//	BPM / 60 = BPS, 1/BPS = Beat period in seconds
	//	Beat period * 4/ frac = delay required in seconds
	//	Delay required / 32us = counts required
	//	Counts / 256 = Overflows required
	//	Counts % 256 = Remainder
	//	In summary:
	//		60 * 31.25kHz * 4
	//		------------- = counts required
	//		BPM * frac
	count_req = (7500000) / (bpm * frac);
	overflow_req = count_req / 256;
	remainder = count_req % 256;

	// Begin the overflow sequence 
	CLEAR0;
	CLEAR_OVERFLOW;
	while (overflow_cnt < overflow_req) {
		while(!CHECK_OVERFLOW);		// Wait for overflow
		overflow_cnt++;
		CLEAR_OVERFLOW;	
	}

	// Handle remainder
	CLEAR0;
	SET_TOP0(remainder);
	while (!CHECK0);
	CLEAR0;
	
	return;			
}

void timer_init() 
{
	// Timer 1 is used for the actual notes (variable frequency PWM)
        TCCR1A = (0xC0);        // Set on compare upcounting, clear on compare downcounting
        TCCR1B = (0x11);        // PWM Phase & Frequency Corrected, /8 prescaler

	OCR1AH = 0;
	OCR1AL = 0x0F;
	ICR1H = 0x00;
	ICR1L = 0x70; 

	// Timer 1 controls PC6, which is hooked up to the buzzer
        DDRC   |= (1 << PC6);        // PB6 -> Output (PWM)
        PORTC  |= (1 << PC6);        // Disable internal pull-up (need smaller resistance for the buzzer)

	// Timer 0 keeps time, for controlling and note/rest length
	TCCR0A = (0xC0);	     // Output Compare mode = set, Normal mode
	
	// Select Prescaler to support BPMS 30 - 300, with 16th (BPM/4) and whole (BPM*4) 
	// note length delays (we're assuming 4:4 timing)
	// max BPM period = 30 beats / 60 seconds = 0.5 beats / second = 2 seconds / beat
	//	* 4 (whole note) = 8 second long note
	// min BPM period = 300 beats / 60 seconds = 5 beats / second = 0.2 seconds / beat
	//	/4 (16th note) = 0.05 seconds / beat 
	// 
	// Our max prescaler is 1024, 8MHz / 1024 = 7812.5 Hz, still very fast
	// We'll use 256 for a round frequency of 31.25 kHz. We'll need to track overflows
	// for appropriate delays
	
	TCCR0B = (0x03);			
        OCR0A = 0;             // Clears overflow value of timer
}


/* The following two event handlers are important.  When the USB interface  */
/* gets the event, we need to use the CDC routines to deal with them           */ 


/* When the USB library throws a Configuration Changed event we need to call */
/* CDC_Device_ConfigureEndpoints                                                                    */
void EVENT_USB_Device_ConfigurationChanged(void)
{    CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface); }


/* When we get a Control Request from the USB library we need to call         */
/* CDC_Device_ProcessControlRequest.                                                        */
void EVENT_USB_Device_ControlRequest(void)
{  CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface); }


/** The structure initialization below  is taken directly from the VirtualSerial   *
* example.  Even with the aid of the Documentation I could not figure out all *
* that is going on here.  It bothers me and I would *LOVE* to have someone *
* find more information                                                                                      */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
        {
                .Config =
                        {
                                .ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
                                .DataINEndpoint           =
                                        {
                                                .Address          = CDC_TX_EPADDR,
                                                .Size             = CDC_TXRX_EPSIZE,
                                                .Banks            = 1,
                                        },
                                .DataOUTEndpoint =
                                        {
                                                .Address          = CDC_RX_EPADDR,
                                                .Size             = CDC_TXRX_EPSIZE,
                                                .Banks            = 1,
                                        },
                                .NotificationEndpoint =
                                        {
                                                .Address          = CDC_NOTIFICATION_EPADDR,
                                                .Size             = CDC_NOTIFICATION_EPSIZE,
                                                .Banks            = 1,
                                        },
                        },
        };
