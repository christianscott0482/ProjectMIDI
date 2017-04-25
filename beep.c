/* Christian Auspland & Matthew Blanchard
 * ECE 477
 * Song interpreter
 * Gets a list of frequencies to play at pre-determined length
 */
#include<stdio.h>
#include<windows.h>
#include<sys/proc.h>
#include<sys/systm.h>
#include<sys/param.h>

int main(int argc, char *argv[])
{
	FILE *fd;	//file descriptor
	int freq = 0;	//contains current note frequency
	int quartlen = 0;	//holds length of a quarter note for
				//this song
	int uquart = 0;	//holds the quartlen variable in microseconds
	
	//check correct number of arguments
	if(argc != 2) {
		printf("Usage: executable, frequency/time file");
		return 1;
	}
	
	//opens frequency file for reading
	fd = fopen(argv[1], "r");
	if (fd == NULL) {
		printf("Error: could not open file %s\n", argv[1]);
		return 2;
	}

	//get first line: the quarternote length for this song
	fscanf(fd, "%d", quartlen);

	//quartlen in microseconds
	uquart = quartlen * 1000;

	while(1) {
		fscanf(fd, "%d", freq);
		if (freq == 0) {
			usleep(uquart);
		}
		else {
			beep(freq, quartlen);
		}

		if (feof(fd)) {
			printf("End of file found\n");
			return 0;
		}
	}
}
return 0;

