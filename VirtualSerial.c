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


/* This code is based on Dean Camera's "VirtualSerial.c" from the Demo code included */
/* with the LUFA library.  Unnecessary parts have been removed and the "High Low"     */
/* game has been dropped in largely unmodified.  This is meant to demonstrate a          */
/* minimal USB device on an AVR.  From the AVR perspective the USB is simply          */
/* stdin and stdout (with a CONNECTED flag).  From the PC's perspective the AVR is   */
/* a serial port.  It is a greatly simplified serial port in the sense that baud rate and         */
/* several other settings do not matter.  (The AVR can access these settings, but this  */
/* is not included in this code.)                                                                                           */




#include "VirtualSerial.h"
#include "stdlib.h"
#include "stdio.h"
#include "util/delay.h"


extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;  //at the bottom of this file
// It is essential, but the documentation didn't give me the information I needed to create one
//so I used the one from the example, but I don't feel good about it.


#define CONNECTED (VirtualSerial_CDC_Interface.State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR)

static FILE USBSerialStream;  //This will become stdin and stdout


void do_high_low(int freq);  // This is from the 477 web site
void bruces_usb_init(void);
void timer_init(int sound);
void delayms(int wait);


/* Below is a minimal main function for doing USB communication through a     */
/* Virtual serial port.  the function bruces_usb_init does the minimal initialization*/
/* then the program waits for a connection (PC opens port) before calling a       */
/* function to do whatever needs to be done.  In this case it is a high-low           */
/* guessing game from the 477 web site.  It **IS** important, however that        */
/* whatever program is run, it must spend the majority of its time blocked         */
/* waiting for I/O (e.g., in a call to scanf)                                                              */
int main(void)
{
	int frequency = 0xFF;

        bruces_usb_init();  // get the USB stuff going
	timer_init(frequency);
       	while(1)
        {
               	while(!CONNECTED) //connected means the PC has opened the serial port
               	{
       	            	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        		USB_USBTask();
                }
               	do_high_low(frequency);
        } 
}


/* This code is essentially identical to the high_low_server code that  */
/* is one of the examples on the 477 page.  File I/O, stdin/stdout and  */
/* serial I/O are all incredibly similar and almost interchangeable           */


void do_high_low(int freq)
{
   	//FILE *fp;
	char new_freq[7];
	long int freq_up = 0;
	//char quartlen[4];
	//int beat = 0;

   	//fp=stdout;
   	//fpr=stdin;
	freq_up = freq;

	//scanf("%s", quartlen);
	//beat = atoi(quartlen);	

  	while(1)
  	{ 

     		scanf("%s", new_freq);
		freq_up	= atoi(new_freq);
		if (freq_up > 0){
 			timer_init(freq_up);
			freq_up = 0;
			memset(new_freq, 0, strlen(new_freq));
			//delayms(beat);

		}
		if (freq_up == 0) {
			timer_init(freq_up);
			freq_up = 0;
			memset(new_freq, 0, strlen(new_freq));
			//delayms(beat);
		}
   	}
   
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

void delayms(int wait)
{
	int k = 0;

	for (k = 0; k < wait; k++) {
		_delay_ms(1);
	}
}

void timer_init(int sound) 
{
	int high = 0;
	int low = 0;
	high = (0xFF00 & sound);
	low = (0x00FF & sound);

        TCCR1A |= (0xc0);       //sets stuff
        TCCR1B |= (0x12);       //set prescalar(/256) and finishes PWM init
        OCR1AH &= 0;             //clears bits
	OCR1AL &= 0;
        OCR1AH = 0; //initializes to middle c, roughly 262hz
	OCR1AL = 0x0F;
	ICR1H = high;
	ICR1L = low;
        DDRC = (1<<PC6);        //makes pb7 an output
        PORTC = (1<<PC6);       //disable internal pull-up

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
