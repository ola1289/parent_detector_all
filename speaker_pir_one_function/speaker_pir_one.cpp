#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

#define SOUNDPIN 6
#define PIR 1

int s_interr = 0;
int p_interr = 0;

void sound_interrupt(void)
{
        s_interr = 1;
}

void pir_interrupt(void)
{
        p_interr = 1;
}

int setup()
{       //inicjacja wiringPi i  podpiêcie funkcji przerwania
        if (   wiringPiSetup()==-1 || wiringPiISR(SOUNDPIN, INT_EDGE_BOTH, &sound_interrupt) <0 || wiringPiISR(SOUNDPIN, INT_EDGE_BOTH, &pir_interrupt) <0)
                return -1;
        return 1;
}

int main()
{
        int counter = 0;
        int camera_status = 0;
        int pir_status = 0;

        if (setup())
        {
        	printf("Initliaze camera\n" );
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
					if (p_interr || s_interr)
					{
						printf("Interrupt detected\n");
						s_interr = 0;
						p_interr = 0;

						if (camera_status == 0 || pir_status == 0)
						{
							printf("Start capturing\n" );
							system("pkill -USR1 raspivid");
							camera_status = 1;
							pir_status = 1;
						}
					}
					delay(200); //ominiêcie delay oznacza obci¹¿enie procesora 25%, tu spada do 2%

					if (camera_status || pir_status)
						++counter; //delay after noise detection

					if (counter > 100)
					{
						printf("Stop capturing\n" );
						system("pkill -USR1 raspivid");
						camera_status = 0;
						pir_status = 0;
						counter = 0;
					}
			}
        }
        return 0;
}
