#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

/*
 * Publishes messages for a specific amount of time 
 * in an mqtt broker with a given address
 * 
 */

void stopCoAPPut (int sig) {
	fclose(stdin); /* abort fgets() */
	printf("\nProgramm terminated!\n");
	exit(EXIT_SUCCESS);
}

char * get_current_time_with_ms (void)
{
    struct timeval  tv;
	gettimeofday(&tv, NULL);

	double time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
    
    char * tmp = (char *) malloc (50);
    sprintf(tmp,"%f", time_in_mill);
    //printf("\ntime in mill %s\n", tmp);
    return tmp;
}

/*
 * This will handle connection for each client
 * */
void *execute_command(void *command)
{
	//printf("command %s\n", (char *) command);
	system(command);
}

int main(int argc, char *argv[]) {

	char * address = "localhost";
	char * logfilename = "logfile_send.txt";
	//pthread_t system_thread;
	if(argc > 1) {
		address = argv[1];
	}
	if(argc > 2) {
		logfilename = argv[2];
	}
	if(argc > 3) {
		printf("\nUsage: ./exp_mqtt_pub <address> <logfilename>\n\n");
		printf("<address> : the address of the mosquitto we are publishing to\n");
		printf("<logfilename> : the name of the logfile to store the received messages\n");
		exit(0);
	}

	signal(SIGINT, stopCoAPPut);

	remove(logfilename);
	FILE * log = fopen(logfilename, "a");

    time_t start, end;
    double elapsed;  // seconds
    double old_elapsed = 0;
	double finish_time = 60*60*2; // 5 seconds
	//printf("finish_time: %f\n", finish_time);
	time(&start);
	int sequential_number = 0;
	char message[1024];
	int rate = 100000;//60000000; //microseconds 1000000 = 1 second, so 100000 = 100ms and 1000 = 1ms
	//printf("rate: %d\n", rate);
	do {
		time(&end);
		elapsed = difftime(end, start);
		sprintf(message, "standard test with fixed message size:%d", sequential_number);
		printf("Elapsed time: %f\n", elapsed);
		printf("message: %s\n", message); // 39 bytes
		char cmd[2048];
		char * timestamp = get_current_time_with_ms();
		char delim[] = ".";
		strtok(timestamp, delim);
		sprintf(cmd, "echo '%s:%s' | coap-client -N -m put coap+tcp://[%s]/iot -f -", message, timestamp, address);
		/*if( pthread_create( &system_thread , NULL ,  execute_command , (void*) cmd) < 0)
		{
			char error[128];
			sprintf(error, "could not create thread %d", sequential_number);
			perror(error);
			return 0;
		}*/		
		system(cmd);
		/*char * timestamp = get_current_time_with_ms();
		char delim[] = ".";
		strtok(timestamp, delim);*/
		//printf("\ntimestamp: %s\n", timestamp);
		sprintf(timestamp, "%s\n", timestamp );
		fputs( timestamp, log);
		fflush(NULL);
		usleep(rate); // wait a second before sending more messages, to wait 100ms, is the same as 1000ms/10, or just take a 0 out, so for 100ms, we take a 0 out from 1000000 (100000) and so on
		//sleep(rate);
		sequential_number++;
		
	}while(elapsed < finish_time);
	printf("Experience finished.\n");

	/* close */
	pclose(log);
	return 0;
}