Main: main.o mqtt.o stopwatch.o backup.o
	g++ -o Main main.o mqtt.o stopwatch.o backup.o -l paho-mqtt3cs -l wiringPi -I/usr/local/include -ltft_st7735

main.o: main.cpp
	g++ -c main.cpp

mqtt.o: mqtt.c mqtt.h
	gcc -c mqtt.c

stopwatch.o: stopwatch.c stopwatch.h
	gcc -c stopwatch.c

backup.o: backup.c backup.h
	gcc -c backup.c

clean: Main
	rm -f *.o

test: Main Test
	./Test

Test: test.o
	gcc -o Test test.o -l cmocka

test.o:	test.c
	gcc -c test.c
