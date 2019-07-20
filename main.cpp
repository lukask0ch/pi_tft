/*****************************************************************************************************
* TFT1_8 1,8" Display from Sainsmart with a ST7735 controller and a RaspberryPi
*
* This is a test program to control the 1.8" ST7735 TFT with the RaspberryPi and tft_st7735 library along
* with the wiringPi library
*/

#include "wiringPi.h"
#include "wiringPiSPI.h"

#include "tft_st7735.h"
#include "tft_manager.h"
#include "tft_field.h"

#include <time.h>
#include <sys/time.h>

#include <wiringPiI2C.h>
#include <unistd.h>
#include<string.h>

#include <iostream>
#include <fstream>

#define BUTTON 18
#define DEBOUNCETIME 15

using namespace std;

float getsoc();
float getvolt();
unsigned long long getMicrotime();

int fd=0;
bool flagbuttonPressed = false;
bool flagshutdown = false;
unsigned long long int interrupt_time =0;

PI_THREAD (buttonThread)
{
	int buttonPressed = 0;
	while(!flagshutdown){
		if(digitalRead(BUTTON) == 1){
			//printf("%d\n", buttonPressed);
			buttonPressed++;
		}
		else{
			buttonPressed = 0;
		}
		if(buttonPressed == DEBOUNCETIME){
			interrupt_time = getMicrotime();
			flagbuttonPressed = true;

			printf("Single Pressed \n");

			for(int i=0;i<500;i++)
			{
				if(digitalRead(BUTTON) == 1)
					buttonPressed++;
				usleep(10000);
			}
			if(buttonPressed>=500)
			{
				flagshutdown = true;
				
			}
		}
		usleep(1000);
	}
	return 0;
}


int main (void)
{
	TFT_ST7735 tft = *new TFT_ST7735(0, 24, 25, 32000000);

	wiringPiSetupGpio();      // initialize wiringPi and wiringPiGpio

	fd = wiringPiI2CSetup(0x36);

	wiringPiI2CWriteReg16(fd, 0x06, 0x4000);

	tft.commonInit();         // initialize SPI and reset display
	tft.initR();              // initialize display
	tft.setBackground(TFT_BLACK);

	tft.clearScreen();        // reset Display
	// tft.setRotation(true);

	tft.drawString(5,2,"Bahn1",TFT_WHITE,2);

	tft.drawString(15,40,"laufende Zeit:",TFT_WHITE,1);
	tft.drawString(15,85,"letzte Zeit:",TFT_WHITE,1);

	tft.drawHorizontalLine(0, 158, 128, TFT_RED);
	tft.drawHorizontalLine(0, 159, 128, TFT_RED);

	pinMode(BUTTON, INPUT);
	int x = piThreadCreate (buttonThread) ;
	if (x != 0)
  		printf ("it didn't start \n");

	char soc[5];
	char volt[5];

	int min, sec, millisec=0;
	unsigned long long int start, timenow = 0;
	char runningtime[10] = "00:00:00";
	char roundtime[10] = "00:00:00";
	int sec_merker=0;

	time_t rawtime;
 	struct tm * timeinfo;
   	char buffer [10];

	int counter = 4390;

	std::ofstream myfile;
	myfile.open ("/home/pi/pi_tft/timesoc2.txt", std::ios_base::app);       //Dateipfad anpasssen!
	myfile << "Neuer Start" << std::endl;
	myfile.close();

	printf("Running\n");

	start = getMicrotime();

	while(1)
	{

		if(counter==4400)
		{
			counter =0;
			//printf("Count \n");

			time (&rawtime);
			timeinfo = localtime (&rawtime);

			strftime (buffer,80,"%H:%M:%S",timeinfo);

			myfile.open ("/home/pi/pi_tft/timesoc2.txt", std::ios_base::app);
			myfile << buffer << " " << getsoc() << " " << getvolt() << std::endl;
			myfile.close();
		}
		counter++;
		
		if(flagbuttonPressed){ //Button pressed
			flagbuttonPressed=false;

			timenow = interrupt_time-start;
			
			min = timenow / 60000;
			sec = (timenow / 1000) % 60;
			millisec = (timenow / 10) % 100;
			sprintf(roundtime, "%02d:%02d:%02d", min, sec, millisec);

			tft.drawString(15,100,roundtime,TFT_WHITE,2);
		}
		if(flagshutdown)
		{
			printf("Long Pressed \n");
			tft.clearScreen(TFT_RED);
			while(digitalRead(BUTTON))
			{
				usleep(10000);
			}
			tft.clearScreen(TFT_GREEN);
			printf("Shutdown \n");
			sleep(2);
			tft.clearScreen();
			flagshutdown=false;
			sleep(1);
			system("sudo shutdown -h now");
			sleep(10);

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
			snprintf(soc,6, "%f %%", getsoc());
			strcat(soc, " %");
			tft.drawString(5,145,soc,TFT_WHITE,1);

			snprintf(volt,6, "%f V", getvolt());
			strcat(volt, " V");
			tft.drawString(70,145,volt,TFT_WHITE,1);
		}
		if (sec>0)
		{
			sec_merker=0;
		}

		delay (120);

	}

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

unsigned long long getMicrotime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	unsigned long long millis = (unsigned long long) currentTime.tv_sec * 1000 + currentTime.tv_usec/1000;
	return millis;
}
