/* Matthew Blanchard & Christian Auspland
 * ECE 477
 * AVR Pin Manager user program
 * Calls upon a USB serial connection to the AVR to 
 * read and modify pin values
 */

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "frequency_codev2.h"


#define BAUDRATE B9600
#define C 0
#define D 2
#define E 4
#define F 5
#define G 7
#define A 9
#define B 11

//void readSerial(int file);

int main(int argc, char *argv[])
{
	int fd = 0;		// File descriptor for serial port
	FILE *fb = 0;		// File descriptor for song
	struct termios usb;     // Termios struct	
	char sound[7];		// holds stdout string
	char end[] = "\r\n";	// end characters to send to AVR
	char bpm[4];		// Length of quarter note in a song
	char note_count[6];	// Number of notes in song; read from file
	int ncount = 0;		// Interger note_count
	char read[4];		// Reads note info
	char note = '0';	// Read note
	char octave = '0';	// Read octave
	int octaven = 0;	// Octave as integer
	char length[2];	// Read note length
	int frequency = 0;	// Frequency from table
	int beat = 0;

	// Expecting two arguments: a device file and an output file
	if (argc != 3) {
		printf("Error: expected two arguments\n");
		return 1;
	}

	// Open the device file
	printf("Opening device file %s ...\n", argv[1]);
	fd = open(argv[1], O_RDWR);
	if (fd <= 0) {
		printf("Error: failed to open device file %s\n", argv[1]);
		return 2;
	}
	printf("Device file opened ... \n");


	// Open song file
	printf("Opening song file %s ...\n", argv[2]);
	fb = fopen(argv[2], "r");
	if (fb == NULL) {
		printf("Error: failed to open song file %s\n", argv[1]);
		return 3;
	}

	// Verify that a TTY device was opened
	if(!isatty(fd)) {
		printf("Error: device file %s is not a TTY device\n", argv[1]);
		return 4;
	}

	// Clear termios struct
	memset(&usb, 0, sizeof(usb));

	// Read existing serial parameters
	if (tcgetattr(fd, &usb) != 0) {
	 	printf("Error: failed to retrieve serial port attributes\n");
	 	return 5;
	}

	// Configure serial port //
	///////////////////////////

	// Transmission speeds
	cfsetospeed(&usb, B9600);	// Output baud 9600 bps
	cfsetispeed(&usb, B9600);	// Input baud 9600 bps

	// Line flags
	usb.c_lflag |= ICANON;	// Canonical mode

	// Character flags
	usb.c_cflag |= CREAD;	// Enable receiver

	usb.c_cflag &= ~PARENB; // Disable parity
	usb.c_cflag &= ~CSTOPB; // One stop bit

	usb.c_cflag &= ~CSIZE;  // Character size is 8	
	usb.c_cflag |= CS8;	

	// Update serial device parameters
	if (tcsetattr(fd, TCSANOW, &usb) != 0) {
		printf("Error: failed to set serial port attributes\n");
		return 6;
	}

	// Clear any preexisting data on the serial interface (input and output)
	tcflush(fd, TCIOFLUSH);
	
	// Get first line: the quarternote length for this song
	fscanf(fb, "%s", bpm);
	write(fd, bpm, strlen(bpm));
	printf("After write, before read");
	//readSerial(fd);
	beat = atoi(bpm);
	printf("Beat sent to AVR: %d\n", beat);
	
	fscanf(fb, "%s", note_count);
	write(fd, note_count, strlen(note_count));
	//readSerial(fd);
	ncount = atoi(note_count);
	printf("Number of notes: %d\n", ncount);



	// Begin transmission

	while(1) {
		printf("start of scan\n");
		if((fscanf(fb, "%s", read) == EOF)) {
			printf("End of file reached\n");
			break;
		}
		note = read[0];
		octave = read[1];
		length[0] = read[3];
		octaven = octave - '0';
		switch (note) {
			case 'A':
				frequency = note_lookup[A][octaven];
				break;
			case 'B':
				frequency = note_lookup[B][octaven];
				break;
			case 'C':
				frequency = note_lookup[C][octaven];
				break;
			case 'D':
				frequency = note_lookup[D][octaven];
				break;
			case 'E':
				frequency = note_lookup[E][octaven];
				break;
			case 'F':
				frequency = note_lookup[F][octaven];
				break;
			case 'G':
				frequency = note_lookup[G][octaven];
				break;
			case '0':
				frequency = 0;
				break;
			default:
				break;
		}
		sprintf(sound, "%d", frequency);
		strcat(sound, end);
		strcat(length, end);
		write(fd, sound, strlen(sound));
		write(fd, length, strlen(length));
		usleep(250000);
		//readSerial(fd);
		//printf("Read note:%c\n", note);
		//printf("Read octave:%c\n", octave);
		//printf("Read length:%c\n", length);
		//printf("Looked up frequency:%d\n\n", frequency);
		/*if ((inote >= 0) && (inote < 65535)) {
			printf("after function note value: %d\n", cnote);
			sprintf(sound, "%d", cnote);
			strcat(sound, end);
			printf("your number is: ");
			for (i = 0; i < 7; i++) {
				printf("%c", sound[i]);
			}
			printf("\n");
			printf("before write\n");
			write(fd, sound, strlen(sound));
			printf("after write\n");
			memset(sound, 0, strlen(sound));
			printf("after memset\n");
			usleep(beat * 1000);
			printf("after sleep\n\n");
		}
		else {
			printf("Value out of range\n");
		}
		if (feof(fb)) {
			printf("End of file reached\n");
			break;
		}*/		
	}
	return 0;

}

/*void readSerial(int file)
{
	char c = 0;
	int j = 0;
	char buf[6];

	while  (1){
		j = 0;
		printf("before read\n");
		read(file, &c, 1);
		printf("after read\n");
		if(c == '\r') break;
		buf[j] = c;
		j++;
	}
	printf("%s\n", buf);
}*/
