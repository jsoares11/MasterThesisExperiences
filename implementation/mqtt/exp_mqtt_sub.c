#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#define BUFFERSIZE 2048

void stopMqttGet (int sig) {
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

int main(int argc, char *argv[]) {

	char * lastSeen;
	char * address = "localhost";
	char buffer[BUFFERSIZE];
	FILE *fp;
	char * logfilename = "logfile_receive.txt";
	char cmd[2048];
	char * line; 

	if(argc > 1) {
		address = argv[1];
	}
	if(argc > 2) {
		logfilename = argv[2];
	}
	if(argc > 3) {
		printf("\nUsage: ./exp_mqtt_sub <address> <logfilename> <variable_size> \n\n");
		printf("<address> : the address of the mosquitto we are subscribing to\n");
		printf("<logfilename> : the name of the logfile to store the received messages\n");
		exit(0);
	}

	remove(logfilename);
	FILE * log = fopen(logfilename, "a");
	
	signal(SIGINT, stopMqttGet);
	sprintf(cmd, "mosquitto_sub -h %s -t iot -q 2 -p 1884", address);
	fp = popen(cmd, "r");

	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}
	/* Read the output a line at a time - output it. */
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		//printf("Received message: %s \n", buffer);
		char * current_ts = get_current_time_with_ms();
		char delim1[] = ".";
		strtok(current_ts, delim1);
		//printf("\ncurrent_ts: %s\n", current_ts);
		char * msg_tmp;
		char * msg;
		char msg_quotes[2048];
		char * msg_sq_num;
		char delim2[] = ":";
		char *token;
		char logged_message[2048];
		char * sender_tmp;

		token = strtok(buffer, "{");
		//printf("token: %s\n", token);
		token = strtok(token, "}");
		//printf("token: %s\n", token);

		msg_tmp = strtok(token, delim2);
		msg = strtok(NULL, delim2);
		msg_sq_num = strtok(NULL, delim2);
		sender_tmp = strtok(NULL, delim2);

		msg_tmp = strtok(msg_tmp, "\"");
		//printf("msg_tmp: %s\n", msg_tmp);
		msg = strtok(msg, "\"");
		//printf("msg: %s\n", msg);
		sprintf(msg_quotes, "\"%s\"", msg);
		//printf("msg_quotes: %s\n", msg_quotes);
		msg_sq_num = strtok(msg_sq_num, "\"");
		//printf("msg_sq_num: %s\n", msg_sq_num);

		sender_tmp = strtok(sender_tmp, "\"");
		//printf("sender_tmp: %s\n", sender_tmp);

		sprintf(logged_message, "%s %s %s %s %s\n", current_ts, msg_tmp, msg_quotes, msg_sq_num, sender_tmp);
		//printf("logged_message: %s\n\n", logged_message );

		fputs( logged_message, log);
		fflush(NULL);
		memset(logged_message, '\0', BUFFERSIZE);
	}

	/* close */
	pclose(log);
	pclose(fp);

	return 0;
}
