#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

#define BUFFERSIZE 2048

/* A script that monitors node status of a kubernetes cluster
# if a node status is "Not Ready" for more then 5 minutes, the script evicts it's pods
# If the node never returns, it is the admin responsability to delete it from the cluster
# this script doesn't delete the node, only starts the eviction of it's pods. If the node does come back to
# life the eviction will proceed and the node will be cordoned, hence to use it again you must uncordon
*/

void stopNodeMonitor (int sig) {
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
void *execute_command(void *nodename)
{
	//printf("nodename %s\n", (char *) nodename);
	if(strcmp((char*) nodename, "slave4") == 0) {
		//printf("ENTREI NA THREAD\n");
		char command[BUFFERSIZE];
		//printf("nodename %s\n", (char *) nodename);
		sprintf(command, "kubectl drain %s --force --ignore-daemonsets  --delete-local-data", (char *) nodename);
		//printf("command %s\n", command);
		system(command);
		printf("Pods were evicted from node%s\n", (char *) nodename);
	}
}

/**
*
*	This function only compares as maximum the days of the dates
* 	If dates are from different months this wont work
*
*/
int is_node_dead(char * heartbeat, char* currentTime){

	char delim2[] = " ";
	char delim3[] = ":";

	strtok(heartbeat, delim2);
	char * day = strtok(NULL, delim2);
	//printf("day %s\n",  day);
	strtok(NULL, delim2);
	char * year = strtok(NULL, delim2);
	//printf("year %s\n", year);
	char * hour = strtok(NULL, delim3);
	//printf("hour %s\n", hour);
	char * min = strtok(NULL, delim3);
	//printf("minutes %s\n", minutes);
	char * sec = strtok(NULL, delim3);
	//printf("seconds %s\n", seconds);

	int heartbeat_in_secs = atoi(hour)*60*60 + atoi(min)*60 + atoi(sec);
	//printf("heartbeat_in_secs %d\n", heartbeat_in_secs);

	// Time away:
	strtok(currentTime, delim2);
	char * current_day = strtok(NULL, delim2);
	//printf("current_day %s\n",  current_day);
	strtok(NULL, delim2);
	char * current_year = strtok(NULL, delim2);
	//printf("current_year %s\n", current_year);
	char * current_hour = strtok(NULL, delim2);
	//printf("current_hour %s\n", current_hour);
	char * current_min = strtok(NULL, delim2);
	//printf("current_min %s\n", current_min);
	char * current_sec = strtok(NULL, delim2);
	//printf("current_sec %s\n", current_sec);

	int currentTime_in_secs = atoi(current_hour)*60*60 + atoi(current_min)*60 + atoi(current_sec);
	//printf("currentTime_in_secs %d\n", currentTime_in_secs);

	int difference;
	if(currentTime_in_secs < heartbeat_in_secs)
		difference = (24*60*60 - heartbeat_in_secs) + currentTime_in_secs;
	else
		difference = (currentTime_in_secs - heartbeat_in_secs)/60;

	//printf("diferença: %d segundos ou seja %d minutos\n", difference, difference/60);

	if(current_day > day) {
		return 1;
	}
	else if(current_hour > hour) {
		return 1;
	}
	else if(difference > 6) {
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	signal(SIGINT, stopNodeMonitor);

	FILE *fp;
	FILE *fp2;
	char buffer[BUFFERSIZE];
	char buffer2[BUFFERSIZE];
    time_t start, end;
    double elapsed;  // seconds
	double finish_time = 60*60*2; // 5 seconds
	time(&start);
	double rate = 6*60*1000000; //microseconds 1000000 = 1 second, so 100000 = 100ms and 1000 = 1ms
	//printf("rate: %d\n", rate);
	
	if(argc > 1) {
		finish_time = atoi(argv[1]);
		//printf("finish_time: %f\n", finish_time);
	}
	if(argc > 2) {
		rate = atoi(argv[2]) * 1000000.0;
		//printf("rate: %f\n", rate);
	}
	if(argc > 3) {
		printf("\nUsage: ./node_monitor <finish_time> <rate>\n\n");
		printf("<finish_time> : the amount to time to check on nodes (in seconds)\n");
		printf("<rate> : the time between each check (in seconds) \n");
		exit(0);
	}

	/*
	# To-do: Delete this comments
	# Ideia: 
	# 
	# Correr script 2h onde a cada 6 minutos faço "get nodes" (porque heartbeat = 5min)
	# 
	# Nesse get nodes tenho que ir linha a linha ver se está "ready" ou "not ready"
	# 
	# Se estiver "not ready" tenho que fazer eviction
	*/
	while(elapsed < finish_time) {
		usleep(rate); // we dont need to get the nodes every time
		time(&end);
		elapsed = difftime(end, start);
		char cmd[2048];
		char cmd2[2048];
		sprintf(cmd, "kubectl get nodes -o wide | awk '{if(NR>2) print $1 \" \" $2}'");
		fp = popen(cmd, "r");
		if (fp == NULL) {
			printf("Failed to run command\n" );
			printf("Errno: %s", strerror(errno));
			//exit(1);
		}
		
		printf("\nElapsed time: %f\n", elapsed);

		if(elapsed >= rate/1000000) {
			// Check what nodes are down every 10 seconds
			printf("elapsed: %f -> checking nodes\n\n", elapsed);
			while((fgets(buffer, sizeof(buffer), fp) != NULL)) {
				//printf("buffer : %s \n", buffer);

				char delim[] = " ";
				
				char * slave = strtok(buffer, delim);
				char * status = strtok(NULL, delim);

				status[strlen(status)-1] = '\0';
				//printf("status: %s\n", status);
				//printf("slave: %s\n", slave);

				// Node is NotReady, so we need to check for how long
				// In our case we evict the pods if it's NotReady for more then 6minutes
				if(strcmp(status, "NotReady") == 0){
					//printf("slave: %s not ready\n", slave);
					sprintf(cmd2, "kubectl describe node %s | grep \"Ready\" | awk '{ if(NR <= 1) print $3 \" \" $4 \" \" $5 \" \" $6 \" \" $7}'", slave);
					//printf("cmd2: %s\n", cmd2);
					fp2 = popen(cmd2, "r");

					if (fp2 == NULL) {
						printf("Failed to run command2\n" );
						printf("Errno: %s", strerror(errno));
						//exit(1);
					}

					if(fgets(buffer2, sizeof(buffer2), fp2) != NULL) {
						// Wed, 09 Sep 2020 14:41:51
						//printf("lastHeartBeat: %s\n", buffer2);

						// Current time:
						char currentTime[BUFFERSIZE];
						time_t rawtime;
						struct tm *info;
						time( &rawtime );
						info = localtime( &rawtime );

						strftime(currentTime, sizeof(currentTime), "%a, %d %b %Y %H %M %S", info);
						//printf("currentTime: %s\n", currentTime);

						// 0- node alive || 1 - node is dead
						int isNodeDead = is_node_dead(buffer2, currentTime);
						//printf("is node dead: %d\n", isNodeDead);

						// if node is dead, then we must evict it's pods
						// since eviction might wait for the node to comeback
						// we make use of threads
						pthread_t command;
						char slave_cpy[BUFFERSIZE];
						sprintf(slave_cpy, "%s", slave);
						if(isNodeDead==1) {
							//printf("Evicting slave %s\n", slave);
							if( pthread_create( &command , NULL ,  execute_command , (void*) slave_cpy) < 0)
							{
								//free(slave);
								perror("could not create thread");
								return 0;
							}
							sleep(1); // apparently pthread_create took so long to execute that slaves were not being evicted.
						}
					}
					//printf("\n");
				}
			}
		}
	}
	printf("Experience finished.\n");

	return 0;
}