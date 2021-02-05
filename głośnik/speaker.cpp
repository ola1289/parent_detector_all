#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

#define SOUNDPIN 6

int zgloszenie=0;

void przerwanie(void)
{
        zgloszenie = 1;
}

int setup()
{       //inicjacja wiringPi i  podpiêcie funkcji przerwania
        if (   wiringPiSetup()==-1 || wiringPiISR(SOUNDPIN, INT_EDGE_BOTH, &przerwanie) <0)
                return -1;
        return 1;
}

int main()
{
        int licznik=0;
        int camera_status = 0;
        if (setup())
        {
        	printf("Initliaze camera\n" );
        	//wlaczenie kamery wp³ywa na input z mikrofonu?
        	system("raspivid -s -t 0 -b 8000000 -o /home/pi/win_share/cam_01.h264 &");

        	FILE *cmd = popen("pgrep raspivid", "r");
			char result[24]={0x0};
			int raspivid_pid = 0;
			fgets(result, sizeof(result), cmd);
			raspivid_pid = atoi(result);
			printf("raspivid_pid = %d\n", raspivid_pid);
			pclose(cmd);

			if(raspivid_pid > 0)
			{
				system("pkill -USR1 raspivid"); //send signal to stop capturing
				printf("camera ok\n" );
			}
			else
			{
				printf("camera failure\n" );
				return 0;
			}


			while(1)
			{
					if (zgloszenie)
					{
						printf("Noise detected\n");
						zgloszenie = 0;
						if (camera_status == 0)
						{
							printf("Start capturing\n" );
							system("pkill -USR1 raspivid");
							camera_status = 1;
						}
					}
					delay(200); //ominiêcie delay oznacza obci¹¿enie procesora 25%, tu spada do 2%

					if (camera_status)
						++licznik; //delay after noise detection

					if (licznik > 100)
					{
						printf("Stop capturing\n" );
						system("pkill -USR1 raspivid");
						camera_status = 0;
						licznik = 0;
					}
			}
        }
        return 0;
}






/*
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <wiringPiI2C.h>
#include <time.h>
#include <iomanip>
#include <thread>
#include <pthread.h>
#include <memory>
#include <unistd.h>
#include <signal.h>


#define SOUNDPIN 6
#define TIMEOUT 5
#define PIR 1
//#define DEVICE_ID 0x18
//#define TEMPPIN 6

using namespace std;

//unsigned int pir_state = 0;
unsigned int pir_data  = 0;
unsigned int sound_state = 0;
unsigned int sound_data  = 0;


struct sigaction sig_struct;

void  finish(int sig)
{
	printf("Program przerwany: %d\n", sig);
	exit(0);
}

void interrupt_move_sensor()
{
	pir_data = digitalRead(PIR);
}

void interrupt_sound_detector(void)
{
	sound_data = 1;
}

int setup(void)
{
	if (wiringPiSetup() ==-1 || wiringPiISR(SOUNDPIN, INT_EDGE_BOTH, &interrupt_sound_detector) <0)
	{
		printf("EXIT" );
			exit(1);
	}
}


int main(int argc, char **argv)
{
	int counter = 0;
	setup();


	printf("Noise detected" );
	//OBSLUGA Ctrl+C
	sig_struct.sa_handler = finish;
	sigemptyset(&sig_struct.sa_mask);
	sig_struct.sa_flags = 0;
	sigaction(SIGINT,&sig_struct,NULL);

	//podlaczenie trybu pinow
	//pinMode(PIR, INPUT);
	//pinMode(SOUNDPIN, INPUT);
	while((sound_data = digitalRead(SOUNDPIN)) != 0)
	{
		usleep(100000);
	}
	printf("spi...\n");
	printf("Czuwanie...\n");

	while (1)
	{
		//PÄ™tla nic nie robi, obsÅ‚uguje przerwanie, obciÄ…Å¼enie procesora bardzo maÅ‚e
		usleep(1000000);
	}

    while(1)
    {
            if (sound_data)
            {
            	printf("Noise detected" );
            	if (system("pgrep raspivid") == 1)
            	{
            		printf("start capturing\n" );
            		system("raspivid -s -t 0 -b 8000000 -o /home/pi/win_share/cam_01.h264");
            	}

                sound_data = 0;
            }
            delay(200); //ominiêcie delay oznacza obci¹¿enie procesora 25%, tu spada do 2%
    }



	return EXIT_SUCCESS;


	//return 0;
}
*/
