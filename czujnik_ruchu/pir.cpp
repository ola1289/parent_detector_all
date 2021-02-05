#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>

//WPi nr 1 / GPIO 18
#define SENSOR 1

//jeśli nie 12 to 26
#define LED_PIN 26


unsigned int state = 0;
unsigned int data  = 0;
struct sigaction sig_struct;

void  finish(int sig)
{
	printf("Program przerwany: %d\n", sig);
	exit(0);
}

void interrupt_move_sensor()
{
	data = digitalRead(SENSOR);
	if(data == 1 && state == 0)
	{
		printf("Ruch - %d\n",(int)time(NULL));
		digitalWrite(LED_PIN, 1);
		state = 1;
	}
	else if (data == 0 && state == 1)
	{
		printf("Czuwanie...\n");
		digitalWrite(LED_PIN, 0);
		state = 0;
	}
}


int main(int argc, char **argv)
{
	if (wiringPiSetup() == -1 || wiringPiISR(SENSOR, INT_EDGE_BOTH, &interrupt_move_sensor) <0) //podlacz przerwanie!!
	{
		printf("Uruchom jako root!\n");
		return EXIT_FAILURE;
	}

	//OBSLUGA Ctrl+C
	sig_struct.sa_handler = finish;
	sigemptyset(&sig_struct.sa_mask);
	sig_struct.sa_flags = 0;
	sigaction(SIGINT,&sig_struct,NULL);

	//podlaczenie trybu pinow
	pinMode(SENSOR, INPUT);
	pinMode(LED_PIN, OUTPUT);

	while((data = digitalRead(SENSOR)) != 0)
	{
		usleep(100000);
	}
	printf("Czuwanie...\n");
	while (1)
	{
		//Pętla nic nie robi, obsługuje przerwanie, obciążenie procesora bardzo małe
		usleep(1000000);
	}
	return EXIT_SUCCESS;
}
