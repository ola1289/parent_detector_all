#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	wiringPiSetupGpio();
	pinMode(17,INPUT);
	pullUpDnControl(17, PUD_DOWN);

	for(;;)
	{
		if(digitalRead(17) == HIGH)
		{
			printf("ujecie:)\n");
			system("raspivid -o /home/pi/win_share/zdjecie2.h264");
		}
	}
	
	return 0;
}

