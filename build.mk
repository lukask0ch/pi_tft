Main: main.o mqtt.o stopwatch.o backup.o
	gcc -o Main main.o mqtt.o stopwatch.o backup.o -l paho-mqtt3cs -l wiringPi -l pthread

main.o: main.c
	gcc -c main.c

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
