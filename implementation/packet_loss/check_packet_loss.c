#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

/*
 * Checks how many packets were lost given a text file with the sequential numbers
 * The result is also shown in percentage -> abandoned because we need to be sure how many msgs were sent
 */


void stopCheckPacketLoss (int sig) {
	fclose(stdin); /* abort fgets() */
	printf("\nA sair do coap_put\n");
	exit(EXIT_SUCCESS);
}

/*float percentage(int total, int lost) {
	float percentage;
	int arrived = total - lost;
	printf("total: %d\n", total);
	printf("arrived: %d\n", arrived);
	percentage = (float) arrived / total * 100.0;\textbf{\textit{Zookeeper Cluster:} }

   //printf("Percentage = %.2f%%", percentage);

   return percentage;
}*/

int main(int argc, char *argv[]) {
	char * file_to_read = "test.txt";
	//char * file_output_name = "packet_loss.txt";
	if(argc > 1) {
		file_to_read = argv[1];
	}
	FILE *fp = fopen(file_to_read, "r");
	//remove(file_output_name);
	//FILE * file_output = fopen(file_output_name, "a");
	const char s[2] = "\n";
	char *piece;
	int i;
	signal(SIGINT, stopCheckPacketLoss);
	int packets_lost = 0;
	int last = 0;
	int sndlast = 0;
	char line[2048];
	if(fp != NULL) {
		while(fgets(line, sizeof line, fp) != NULL ) {
			if(line[0] == '\0' || strlen(line) == 0 || line[0] == '\n')
				continue;
			
			int lost = 0;

			/*printf("\nline %d\n", atoi(line));
			printf("last %d\n", last);
			printf("sndlast %d\n", sndlast);*/

			// We dont count duplicated messages nor 
			// messages that came from outdated kafkas
			if (atoi(line) <= last || ((last == 0) && (sndlast == 0))) {
				sndlast = last;
				last = atoi(line);
				//printf("new sndlast %d\n", sndlast);
				//printf("new last %d\n", last);
				continue;
			}

			if(last < sndlast) {
				if( (atoi(line) - sndlast > 2) ){
					lost += atoi(line) - (sndlast+1);				
					printf("line %d\t lost %d\n", atoi(line), lost);
				}
				packets_lost+=lost;
				sndlast = last;
				last = atoi(line);
				continue;
			}
			else if(atoi(line) - last > 1) {
				lost = atoi(line) - last;
				printf("line %d\t lost %d\n", atoi(line), lost);
			}

			/*if(sndlast < last && lost > 0) {
				packets_lost+=lost;
				printf("line %d\t lost %d\n", atoi(line), lost);
			}*/

			packets_lost+=lost;
			
			sndlast = last;
			last = atoi(line);
			//printf("new sndlast %d\n", sndlast);
			//printf("new last %d\n", last);
			//packets_lost++;
		}
		fclose(fp);
	} else {
		perror(file_to_read);
	}
	printf("last: %s\n", line);
	printf("packets_lost: %d\n", packets_lost);

	/* 
	 * Percentage calculation abandoned because we need to be sure how many messages were sent. 
	 * printf("Percentage = %.2f%%\n", percentage(total, packets_lost)); 
	*/ 
	return 0;
}
