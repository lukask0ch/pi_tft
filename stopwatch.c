#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

void getTime(char** returnValue){

	struct timeval tval;
	gettimeofday(&tval, NULL);

	char* str;
	str = malloc(20 * sizeof(char));
	unsigned long long millis = (unsigned long long) tval.tv_sec * 1000 + tval.tv_usec / 1000;
	sprintf(str, "%lld", millis);
	
	strcpy(*returnValue, str);

	free(str);
}

void getDaytime(char** returnValue){

	time_t rawtime;
	struct tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	char* str;
	str = (char *) malloc(32 * sizeof(char));
	sprintf(str, "%02d.%02d.%d %02d:%02d:%02d", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	strcpy(*returnValue, str);

	free(str);
}
