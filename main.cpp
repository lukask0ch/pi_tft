/*****************************************************************************************************
* main.cpp
*/

#include "wiringPi.h"
#include "wiringPiSPI.h"
#include <wiringPiI2C.h>

#include "tft_st7735.h"
#include "tft_manager.h"
#include "tft_field.h"

#include <time.h>
#include <sys/time.h>

#include <unistd.h>
#include <string.h>
#include <math.h>

#include <iostream>
#include <fstream>

extern "C" {
#include "mqtt.h"
#include "stopwatch.h"
#include "backup.h"
}

#define BUTTON 20
#define DEBOUNCETIME 15
#define BAHN "Bahn1"
#define BROKERIP "192.168.213.66:1883"
#define TOPIC "event/timer/1.1/time"
#define TOPIC_S "event/timer/1.1/status"

//using namespace std;

float getsoc();
float getvolt();
float calcSoc();
unsigned long long getMicrotime();

int fd = 0;
bool flagbuttonPressed = false;
bool flagshutdown = false;
unsigned long long int interrupt_time = 0;

PI_THREAD(status)
{
	char* state = (char*) "online";
	while(1)
	{
		sendMessage((char*)TOPIC_S, state);
		sleep(10);
	}
	return 0;
}

PI_THREAD (buttonThread)
{
	int buttonPressed = 0;
	
	char* payload;
	payload = (char *) malloc(20 * sizeof(char));
	
	while(1)
	{
		if(digitalRead(BUTTON) == 0)
			buttonPressed++;
		else
			buttonPressed = 0;
		
		if(buttonPressed == DEBOUNCETIME)		// Single Press
		{
			interrupt_time = getMicrotime();
			
			getTime(&payload);
			sendMessage((char*)TOPIC, payload);
			
			flagbuttonPressed = true;
			writeBackup(payload);

			for(int i=0;i<500;i++)
			{
				if(digitalRead(BUTTON) == 0)
					buttonPressed++;
				usleep(10000);
			}
			if(buttonPressed>=500)				// Long Press
				flagshutdown = true;
		}
		usleep(1000);
	}
	//free(&payload);
	return 0;
}

PI_THREAD (logbattery)
{
	time_t rawtime;
 	struct tm * timeinfo;
   	char buffer [10];
	
	std::ofstream myfile;
	myfile.open ("/home/pi/pi_tft/timesoc2.txt", std::ios_base::app);       //Dateipfad anpasssen!
	myfile << "Neuer Start" << std::endl;
	myfile.close();
	
	while(1)
	{
		time (&rawtime);
		timeinfo = localtime (&rawtime);

		strftime (buffer,80,"%H:%M:%S",timeinfo);

		myfile.open ("/home/pi/pi_tft/timesoc2.txt", std::ios_base::app);
		myfile << buffer << " " << getsoc() << " " << getvolt() << std::endl;
		myfile.close();	
		
		sleep(600);
	}
	return 0;
		
}


int main (void)
{
	TFT_ST7735 tft = *new TFT_ST7735(0, 24, 25, 32000000);
	
	instantiateClient((char*)BROKERIP);
	connectToBroker();

	backupInit();

	wiringPiSetupGpio();      					// initialize wiringPi and wiringPiGpio

	pinMode(BUTTON, INPUT);
	pullUpDnControl(BUTTON, PUD_UP);
	
	fd = wiringPiI2CSetup(0x36);				// initialize I2C Device

	wiringPiI2CWriteReg16(fd, 0x06, 0x4000);	// start I2C Device

	tft.commonInit();         					// initialize SPI and reset display
	tft.initR();              					// initialize display
	tft.setBackground(TFT_BLACK);

	tft.clearScreen();        					// reset Display
	// tft.setRotation(true);					

	tft.drawString(5,2,BAHN,TFT_WHITE,2);

	tft.drawString(15,40,"laufende Zeit:",TFT_WHITE,1);
	tft.drawString(15,85,"letzte Zeit:",TFT_WHITE,1);

	tft.drawHorizontalLine(0, 158, 128, TFT_RED);
	tft.drawHorizontalLine(0, 159, 128, TFT_RED);

	int x = piThreadCreate (buttonThread) ;
	if (x != 0)
  		printf ("buttonThread didn't start \n");
	x = piThreadCreate (status) ;
	if (x != 0)
  		printf ("statusThread didn't start \n");

	char soc[10];
	char volt[10];
	
	float socf, voltf = 0;

	int min, sec, millisec = 0;
	unsigned long long int start, timenow = 0;
	char runningtime[10] = "00:00:00";
	char roundtime[10] = "00:00:00";
	int sec_merker = 0;

	printf("Running\n");

	start = getMicrotime();

	while(1)
	{		
		if(flagbuttonPressed)					// Single Press
		{ 
			flagbuttonPressed=false;

			timenow = interrupt_time-start;
			
			min = timenow / 60000;
			sec = (timenow / 1000) % 60;
			millisec = (timenow / 10) % 100;
			sprintf(roundtime, "%02d:%02d:%02d", min, sec, millisec);

			tft.drawString(15,100,roundtime,TFT_WHITE,2);
		}

		if(flagshutdown)					//Long Press
		{
			tft.clearScreen(TFT_RED);
			tft.drawString(15,40,"Loslassen zum Herunterfahren",TFT_WHITE,2);
			sleep(2);
			if(digitalRead(BUTTON))
			{
				tft.clearScreen(TFT_GREEN);
				sleep(2);
				tft.clearScreen();
				
				sleep(1);
				system("sudo shutdown -h now");
				sleep(10);
			}
			else
			{
				flagshutdown=false;
				tft.clearSreen();
				tft.drawString(5,2,BAHN,TFT_WHITE,2);

				tft.drawString(15,40,"laufende Zeit:",TFT_WHITE,1);
				tft.drawString(15,85,"letzte Zeit:",TFT_WHITE,1);
			}
		}
		
		timenow = getMicrotime()-start;

		min = timenow / 60000;
		sec = (timenow / 1000) % 60;
		millisec = (timenow / 10) % 100;
		sprintf(runningtime, "%02d:%02d:%02d", min, sec, millisec);

		tft.drawString(15,55,runningtime,TFT_WHITE,2);

		if (sec==0 & sec_merker==0)					//update Akkuanzeige einmal pro minute
		{
			sec_merker=1;
			socf = getsoc();				//getsoc or calcSoc
			snprintf(soc,6, "%f", socf);
			strcat(soc, " %");
			tft.drawString(5,145,soc,TFT_WHITE,1);

			voltf = getvolt();
			snprintf(volt,6, "%f", voltf);
			strcat(volt, " V");
			tft.drawString(70,145,volt,TFT_WHITE,1);
		}
		if (sec>0)
			sec_merker=0;

		delay (120);
	}
	disconnectFromBroker();
	
	return 0;
}

float getsoc()
{
	uint8_t soc2;
  	float percent;
 	percent = wiringPiI2CReadReg8(fd, 0x04);
	soc2 = wiringPiI2CReadReg8(fd, 0x05);
  	percent += (float) (((uint8_t) soc2) / 256.0);

  	return percent;
}

float getvolt()
{
	uint16_t vCell1;
	uint16_t vCell2;
  	vCell1 = wiringPiI2CReadReg8(fd, 0x02);
	vCell2 = wiringPiI2CReadReg8(fd, 0x03);
  	vCell1 = (vCell1) << 4;
	vCell2 = (vCell2) >> 4;
	
  	return ((float) (vCell1+vCell2) / 800.0);
}

float calcSoc()
{
	float result, temp, rawsoc, rawvolt;
	
	rawsoc = getsoc();
	rawvolt = getvolt();
	
	temp = (rawvolt-3.48)*100/(4.15-3.48);		// Volt in Prozent umrechen mit max und min Batteriespannung
	//result = sqrt((rawsoc*rawsoc+2*temp*temp)/3);	// gewichteter mittelwert
	result = (rawsoc+temp)/2;			// mitterlwert
	return result;
}

unsigned long long getMicrotime()
{
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	unsigned long long millis = (unsigned long long) currentTime.tv_sec * 1000 + currentTime.tv_usec/1000;
	return millis;
}
