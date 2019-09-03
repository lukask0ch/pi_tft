#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "stopwatch.c"
#include "backup.c"


static long timestamp(){

	char* time = (char *) malloc(20 * sizeof(char));
	char str[10];
	long timestamp;

	getTime(&time);
	timestamp = atol(strncpy(str, time, 10));
	
	free(time);
	return timestamp;
}

static void getTime_test(void **state){

	time_t t;
	time_t g;
	struct tm *tTime;
	struct tm *gTime;

	g = (time_t) timestamp();
	gTime =  localtime(&g);

	time(&t);
	tTime = localtime(&t);

	printf("[ Time     ] getTime_test %s", asctime(gTime));
	assert_string_equal(gTime, tTime);
}

static void getDaytime_test(void **state){

	time_t t;
	struct tm *daytime;
	char* tTime = (char *) malloc(32 * sizeof(char));
	char* gTime = (char *) malloc(32 * sizeof(char));

	getDaytime(&gTime);
	time(&t);
	daytime = localtime(&t);
	sprintf(tTime, "%02d.%02d.%d %02d:%02d:%02d", daytime->tm_mday, daytime->tm_mon+1, daytime->tm_year+1900, daytime->tm_hour, daytime->tm_min, daytime->tm_sec);
	
	printf("[ Time     ] getDaytime %s", tTime);
	assert_string_equal(tTime, gTime);

	free(tTime);
	free(gTime);
}

static int compareFiles(FILE *f1, FILE *f2){

	char c1 = getc(f1);
	char c2 = getc(f2);

	while(c1 != '\n'){ //EOF funktioniert nicht

		if(c1 != c2){
			printf("[  ERROR   ] backupFile File1 != File2");
			return 0;
		}
		c1 = getc(f1);
		c2 = getc(f2);
	}
	return 1;
}

static void backupFile_test(void **state){

	char* daytime = (char *) malloc(32 * sizeof(char));
	char* str = "test";
	FILE* f1;
	FILE* f2;

	getDaytime(&daytime);

	f1 = fopen("testBackupFile.txt", "w");
	f2 = fopen("backupFile.txt", "w");

	fprintf(f1, "%s, %s\n", daytime, str);
	fclose(f1);

	writeBackup(str);

	f1 = fopen("testBackupFile.txt", "r");
	f2 = fopen("backupFile.txt", "r");

	assert_true(compareFiles(f1,f2));

	free(daytime);
	fclose(f1);
	fclose(f2);
}


int main(void){

	printf("\nCheck timestamp!!!\n\n");
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(getTime_test),
		cmocka_unit_test(getDaytime_test),
		cmocka_unit_test(backupFile_test),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
