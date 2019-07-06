# Library for the Raspberry Pi. 

It uses a 1,77" TFT SPI Display and a I2C Fuel Gauge.

## About
Library for the Raspberry Pi and a 1,77" TFT Display based on the library by Juergen Schick (jschick).
Released under the LGPL license.

This library provides basic functions to initialize the ST7735 Controller and draw text, lines and circles
and text fields on the display. Communication between the Raspberry Pi and the ST7735 is managed by the
wiringPi library (see http://projects.drogon.net/raspberry-pi/wiringpi/)

The source code in tft_st7735.cpp/h is based on several examples from the internet, mostly for the arduino.

## Required Hardware
* Raspberry Pi
* 1,8" or 1,77" TFT from Sainsmart or AZDelivery with ST7735 Controller
* I2C Fuel Gage

## Setup
* Install wiringPi. See http://projects.drogon.net/raspberry-pi/wiringpi/
* Checkout this project
* Build it with make [install]. 'Install' will install the library and includes to usr/local/lib and usr/local/include
* Additional make directives: clean install-static uninstall

