#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <wiringPiI2C.h>
#include <time.h>
//#include <bitset>

#define SOUNDPIN 6
#define PIR 1
//i2c
#define DEV_ADDR 0x18
#define TEMP_REG 0x05
#define T_CONF_REG 0x01
#define T_UP_REG 0x02
#define T_DOWN_REG 0x03
#define T_CRIT_REG 0x04

using namespace std;

int sound_interr = 0;
int pir_interr = 0;
float low_temp_limit = -1;
float upp_temp_limit = -1;

float getTemperature(int dev_addr, int reg_addr)
{
	int temp_reg = wiringPiI2CReadReg16(dev_addr, reg_addr);
	int t = temp_reg;
	int M_sb = temp_reg & 0x000f; //clear <15-12> bits
	int L_sb = (t & 0xff00)>>8;

	float temp = (M_sb*16) + static_cast< float >(L_sb) /16;

	return temp;
}

void pir_interrupt(void)
{
	cout<<"move detected!"<<endl;
	pir_interr = 1;
}

void sound_interrupt(void)
{
	cout<<"noise detected!"<<endl;
	sound_interr = 1;
}

int temp_hot(int &handle, int upp_temp_limit)
{
	float current_temp = getTemperature(handle,TEMP_REG);

	if(current_temp >= upp_temp_limit)
	{
		cout << "current_temp = " << current_temp << " upp_temp_limit = " << upp_temp_limit << endl;
		cout<<"Too hot"<<endl;

		return 1;
	}
	else
		return 0;
}

int temp_cold(int &handle, int low_temp_limit)
{
	float current_temp = getTemperature(handle,TEMP_REG);

	if(current_temp <= low_temp_limit)
	{
		cout << "current_temp = " << current_temp << " low_temp_limit = " << low_temp_limit << endl;
		cout<<"Too cold"<<endl;

		return 1;
	}
	else
		return 0;
}

int setup(int &handle, int &fd)
{
		cout << "SETUP STARTED"<<endl;

        if (fd==-1 || handle==-1)
        {
        	cout << "Failed to init WiringPi communication.\n";
            return -1;
        }

    	//write to i2c
        cout << "Temperature Sensor Setup started" << endl;
    	wiringPiI2CWriteReg16(handle, T_UP_REG,0x9101); //upper temp treshold set to 25 C
    	wiringPiI2CWriteReg16(handle, T_DOWN_REG,0x2101); //lower temp treshold set to 18 C

    	upp_temp_limit = getTemperature(handle, T_UP_REG);
    	low_temp_limit = getTemperature(handle, T_DOWN_REG);

    	cout << "Temperature upper limit " << upp_temp_limit << endl;
    	cout << "Temperature lower limit " << low_temp_limit << endl;

    	if (upp_temp_limit != -1 && low_temp_limit != -1)
    	{
    		cout << "Temperature Sensor ok" << endl;
    	}
    	else
    	{
        	cout << "Failed to write to I2C.\n";
            return -1;
    	}

        cout << "Initialize camera" << endl;
        system("bash -c \"(raspivid -s -vf -o - -t 0 -n -w 320 -h 240 -fps 24 &) | (tee -a /home/pi/win_share/test_video.h264 &) | (cvlc -vvv stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8000/}' :demux=h264 &)\"");

    	//checking if raspivid process has been started
    	FILE *cmd = popen("pgrep raspivid", "r");
    	char result[24]={0x0};
    	int raspivid_pid = 0;
    	fgets(result, sizeof(result), cmd);
    	raspivid_pid = atoi(result);
    	cout << "raspivid_pid = " << raspivid_pid << endl;
    	pclose(cmd);

    	if(raspivid_pid > 0)
    	{
        	delay(1000);
    		system("pkill -USR1 raspivid"); //send signal to stop capturing - end of initialization
    		cout << "Camera ok" << endl;
    	}
    	else
    	{
    		cout << "Camera failure" << endl;
    		return -1;
    	}

        if (wiringPiISR(SOUNDPIN, INT_EDGE_RISING, &sound_interrupt) <0
        		|| wiringPiISR(PIR, INT_EDGE_RISING, &pir_interrupt) <0)
        {
        	cout << "Failed to init interrupts.\n";
            return -1;
        }

        cout << "SETUP OK"<< endl << "End of Setup" << endl;
        return 1;
}

int main(int argc, char **argv)
{

	int counter = 0;
	int camera_status = 0;
	int handle = wiringPiI2CSetup(DEV_ADDR);
	int fd = wiringPiSetup();

    if (setup(handle, fd))
    {
    	cout << "Waiting for any event" << endl;
    	while(1)
    	{

    		if (sound_interr || pir_interr ||
    				temp_hot(handle, upp_temp_limit) == 1 || temp_cold(handle, low_temp_limit) == 1)
    		{
    			sound_interr = 0;
    			pir_interr = 0;
    			counter = 0;	//each event clears counter in order to continue recording

    			if (camera_status == 0)
    			{
    				cout << "Start capturing the video" << endl;
    				system("pkill -USR1 raspivid");
    				camera_status = 1;
    			}
    			cout << "Waiting for next event" << endl;
    		}
    		delay(500); //delay for decreasing cpu load

    		if (camera_status)
    			++counter; //camera should run for period of time after last event

    		if (counter > 100)
    		{
    			cout << "Stop capturing the video" << endl;
    			system("pkill -USR1 raspivid");
    			camera_status = 0;
    			counter = 0;
    		}
    	}
    }
    else
    {
    	cout << "SETUP FAILURE" << endl;
    }

    return 0;
}
