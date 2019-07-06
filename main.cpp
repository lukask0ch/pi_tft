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

float getsoc();
float getvolt();
long getMicrotime();

int fd=0;


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
    
  tft.drawHorizontalLine(0, 158, 128, TFT_RED);
  tft.drawHorizontalLine(0, 159, 128, TFT_RED);

  time_t rawtime;
  struct tm * timeinfo;
  char buffer [10];

  int buttonPressed = 0;
  pinMode(18, INPUT);

  char soc[5];
  char volt[5];

  long int start, stop =0;
  start = getMicrotime();

  int counter = 2390;

  std::ofstream myfile;
  myfile.open ("/home/pi/tft/timesoc.txt", std::ios_base::app);
  myfile << "Neuer Start" << std::endl;
  myfile.close();

  printf("Running\n");

  while(1)
  {
	

	time (&rawtime);
  	timeinfo = localtime (&rawtime);
	
 	strftime (buffer,80,"%H:%M:%S",timeinfo);
	tft.drawString(15,40,buffer,TFT_WHITE,2);
	

	snprintf(soc,6, "%f %%", getsoc());
	strcat(soc, " %");
	tft.drawString(5,145,soc,TFT_WHITE,1);

	snprintf(volt,6, "%f V", getvolt());
	strcat(volt, " V");
	tft.drawString(70,145,volt,TFT_WHITE,1);

	if(digitalRead(18) == 1){
		stop = getMicrotime()-start;
		stop = stop * 1e-4;
		printf("%u \n", stop);
	}

	if(counter==2400) //2400 ca 10 min
	{
		counter =0;
		//printf("%s \n", buffer);

		myfile.open ("/home/pi/tft/timesoc.txt", std::ios_base::app);
		myfile << buffer << " " << getsoc() << " " << getvolt() << std::endl;
		myfile.close();
	}
	counter++;
	
	delay (100);

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

long getMicrotime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}


