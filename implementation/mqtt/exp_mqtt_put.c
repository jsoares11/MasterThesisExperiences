#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <sys/time.h>
#include <errno.h> 
#include <pthread.h>

/*
 * Publishes messages for a specific amount of time 
 * in an mqtt broker with a given address
 * 
 */

void stopMqttPut (int sig) {
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
	printf("command %s\n", (char *) command);
	system(command);
}

int main(int argc, char *argv[]) {

	char * address = "localhost";
	char * logfilename = "logfile_send.txt";
	int size = 0; //false
	char logtext[1024];
	pthread_t system_thread;
	int big = -1;

	if(argc > 1) {
		address = argv[1];
	}
	if(argc > 2) {
		logfilename = argv[2];
	}
	if(argc > 3) {
		size = atoi(argv[3]);
	}
	if(argc > 4) {
		printf("\nUsage: ./exp_mqtt_sub <address> <logfilename>\n\n");
		printf("<address> : the address of the mosquitto we are publishing to\n");
		printf("<logfilename> : the name of the logfile to store the messages timestamp\n");
		printf("<size> : 1-fixed (small:39bytes) | 2-fixed (big:2000bytes) | 3- variable (between 100 and 1000 bytes)\n");
		exit(0);
	}

	signal(SIGINT, stopMqttPut);

	remove(logfilename);
	FILE * log = fopen(logfilename, "a");

    time_t start, end;
    double elapsed;  // seconds
	double finish_time = 60*60*2; // 2 hours
	start = time(NULL);
	int terminate = 1;
	int sequential_number = 0;
	char message[2048+1];
	char * payload;
	int len;

	if(size == 2 || size == 3) {
		payload = malloc(2048+1);
	}

	if(size == 2){
		memset(payload, 'a', 2000);
		len = 2000;
		//printf("payload: %s\n", payload);
	}
	
	
	while (terminate) {
		end = time(NULL);
		elapsed = difftime(end, start);

		if(size == 1) { // fixed small
			sprintf(message, "standard test with fixed message size:%d", sequential_number);
			len = strlen(message);
		}
		else if(size == 2) { //fixed big
			sprintf(message, "%s:%d", payload, sequential_number);
		}
		else if(size == 3) { //variable
			srand(time(NULL));
			len = (rand() % (1000 - 100 + 1)) + 100;
			//printf("len %d\n", len);
			memset(payload, 'a', len);
			//printf("variable_message %s\n", variable_message);
			sprintf(message, "%s:%d", payload, sequential_number);
			//printf("message %s\n", message);
		}
		//printf("\nmessage: %s\n", message); // 39 bytes
		//printf("Elapsed time: %f \n", elapsed);

		if (elapsed >= finish_time /* seconds */)
			terminate = 0;
		else {  // No need to sleep when 10 minutes elapsed.
			char cmd[2048];
			char * timestamp = get_current_time_with_ms();
			char delim[] = ".";
			strtok(timestamp, delim);
			sprintf(cmd, "mosquitto_pub -h %s -p 1883 -t iot -q 2 -m '\"%s\":%s'", address, message, timestamp);
			//printf("Sending: %s\n", cmd);
			system(cmd);

			/*if( pthread_create( &system_thread , NULL ,  execute_command , (void*) cmd) < 0)
			{
				char error[128];
				sprintf(error, "could not create thread %d", sequential_number);
				perror(error);
				return 0;
			}*/

			//printf(" Value of errno: %d\n ", errno);

			/*char * timestamp = get_current_time_with_ms();
			char delim[] = ".";
			strtok(timestamp, delim);*/
			//printf("timestamp: %s\n", timestamp);
			//sprintf(timestamp, "%s\n", timestamp );
			//printf("\ntimestamp: %s\n", timestamp);
			//if(size == 3) {
			sprintf(logtext, "%s %d\n", timestamp, len);
			/*}
			else {
				sprintf(logtext, "%s\n", timestamp);
			}*/
			//printf("logtext: %s\n", logtext);
			fputs( logtext, log);
			fflush(NULL);
			usleep(10000); // wait a second before sending more messages, to wait 100ms, is the same as 1000ms/10, or just take a 0 out, so for 100ms, we take a 0 out from 1000000 (100000) and so on
			//sleep(1);
			sequential_number++;
		}
	}
	printf("Experience finished.\n");

	/* close */
	pclose(log);
	if(size == 2 || size == 3) {
		free(payload);
	}
	return 0;
}