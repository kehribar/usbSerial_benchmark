/*--------------------------------------------------------------------
/ This program reads a serial stream from a Virtual Com Port device.
/ It calculates the transfer speed in kbytes/sec. Also, it assumes
/ that the data stream consists of incremental values and checks the
/ integrity of that data stream by comparison. 
/--------------------------------------------------------------------
/ This program is based on the PJRC's serial_listen benchmark app.
/--------------------------------------------------------------------
/ <ihsan@kehribar.me>
/-------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define TEST_SIZE 2500*100

int main(int argc, char **argv)
{	
	int q;
	long n;	
	int port;	
	int i = 0;
	double diff;
	long elapsed;
	long sum = 0;
	int index = 0;
	int errCount = 0;
	unsigned char checkVal;
	unsigned char buf[8192];		
	unsigned char bigBuffer[3200*1000];

	struct timeval t1;
	struct timeval t0;
    struct termios settings;

	if (argc < 2) 
	{
		fprintf(stderr, "Usage:   serial_listen <port>\n");
		return 1;
	}

	port = open(argv[1], O_RDONLY);
	if (port < 0) 
	{
		fprintf(stderr, "Unable to open %s\n", argv[1]);
		return 1;
	}

	tcgetattr(port, &settings);
	cfmakeraw(&settings);
	tcsetattr(port, TCSANOW, &settings);

	FILE* fd = fopen("log.txt","w+");
	
	printf("Reading from %s\n", argv[1]);
	gettimeofday(&t0,0);
	while (1) 
	{
		n = read(port, buf, sizeof(buf));
		if (n < 1) 
		{
			fprintf(stderr, "error reading from %s\n", argv[1]);
			break;
		}
		
		sum += n;
		
		i = 0;
		while(n)
		{
			bigBuffer[index++] = buf[i++];
			n--;
		}
		
		if(sum > TEST_SIZE)
		{
			break;
		}

	}
	gettimeofday(&t1,0);

	checkVal = bigBuffer[0];
	printf("First element: %d\r\n",checkVal);
	for(q=1;q<sum;q++)
	{	
		checkVal++;
		if(bigBuffer[q] != checkVal)
		{
			errCount++;
			fprintf(fd,"Err e:%d r:%d\r\n",checkVal,bigBuffer[q]);
		}
	}
	printf("Err count :%d\r\n",errCount);
	printf("Err percentage: %%%f\r\n",(double)errCount/(double)sum*100.0);

	elapsed = (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec;

	diff = (double)elapsed / 1000000.0;

	printf("Total bytes read: %ld\n", sum);
	printf("Speed %.2f kbytes/sec\n", (float)sum / diff / 1000);

	return 0;
}

