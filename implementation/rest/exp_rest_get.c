#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#define BUFFERSIZE 2048

void stopRESTGet (int sig) {
	fclose(stdin); /* abort fgets() */
	/* close */
	printf("\nProgramm terminated\n");
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

	char lastSeen[2048];
	strcpy(lastSeen, "none");
	int first = 1; // 0-false 1-true
	char * address = "localhost";
	char buffer[BUFFERSIZE];
	char received_msg[BUFFERSIZE];
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
		printf("\nUsage: ./exp_coap_get <address> <logfilename>\n\n");
		printf("<address> : the address of the coap server we are getting data from\n");
		printf("<logfilename> : the name of the logfile to store the received messages\n");
		exit(0);
	}

	remove(logfilename);
	FILE * log = fopen(logfilename, "a");
	
	signal(SIGINT, stopRESTGet);
	sprintf(cmd, "./../../proxy-image/proxy/api-rest/ulfius/example_programs/simple_example/simple_client %s 8000 GET '/mibroker/iot'", address);
	time_t start = time(NULL);
	//printf("start: %ld\n", start);
	double elapsed = 0;
	double timeout = 2*60*60; // 20 seconds receiving the same message we assume the experience is over

	while (elapsed < timeout){
		//usleep(1000);
		fp = popen(cmd, "r");

		if (fp == NULL) {
			printf("Failed to run command\n" );
			exit(1);
		}

		elapsed = (time(NULL) - start);
		//printf("\nelapsed %f\n", elapsed);
		int count = 0;
		
		// the 11th position is where the raw message is
		while (fgets(buffer, sizeof(buffer), fp) != NULL && count != 11) { 

			//printf("\nlastSeen: %s", lastSeen);
			//printf("buffer : %s \n", buffer);
			strcpy(received_msg, buffer);
			count++;
		}

		printf("received_msg %s", received_msg);
		if ( received_msg == NULL || (received_msg != NULL) && (received_msg[0] == '\0')) {
			printf("A mensagem estava vazia\n");
			continue;
		}
		char * current_ts = get_current_time_with_ms();
		char delim1[] = ".";
		strtok(current_ts, delim1);
		//printf("current_ts %s\n", current_ts);

		if(strcmp(lastSeen, received_msg) != 0) { // 0: equal !0: different, if the first element of buffer is \n then the message is empty

			//the received message is new
			//printf("Entrei no if\n\n\n\n");
			start = time(NULL); // start counting for the timeout again
			strcpy(lastSeen, received_msg);
			if(first) {
				first--;
				continue;
			}
			char * msg_tmp;
			char * msg;
			char msg_quotes[2048];
			char * msg_sq_num;
			char * sender_tmp;
			char delim[] = ":";
			char *token;
			char logged_message[2048];

			token = strtok(received_msg, "{");
			//printf("token: %s\n", token);
			token = strtok(token, "}");
			//printf("token: %s\n\n\n\n", token);

			msg_tmp = strtok(token, delim);
			msg = strtok(NULL, delim);
			msg_sq_num = strtok(NULL, delim);
			sender_tmp = strtok(NULL, delim);

			msg_tmp = strtok(msg_tmp, "\"");
			printf("msg_tmp: %s\n", msg_tmp);
			msg = strtok(msg, "\"");
			printf("msg: %s\n", msg);
			sprintf(msg_quotes, "\"%s\"", msg);
			printf("msg_quotes: %s\n", msg_quotes);
			msg_sq_num = strtok(msg_sq_num, "\"");
			printf("msg_sq_num: %s\n", msg_sq_num);

			sender_tmp = strtok(sender_tmp, "\"");
			printf("sender_tmp: %s\n", sender_tmp);

			sprintf(logged_message, "%s %s %s %s %s\n", current_ts, msg_tmp, msg_quotes, msg_sq_num, sender_tmp);
			printf("logged_message: %s\n", logged_message );

			fputs( logged_message, log);
			fflush(NULL);
		}
		else {
			printf("The message received was the same as the before\n");
		}
	}

	//printf("elapsed %f\n", elapsed);
	printf("Timeout reached, experience finished.\n");

	/* close */
	pclose(log);
	pclose(fp);

	return 0;
}
