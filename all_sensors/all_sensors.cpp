#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <wiringPiI2C.h>
#include <time.h>
#include <iomanip>
#include <string>

#define FIXED_FLOAT(x) setprecision(2)<<fixed<<(x)
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

//raspivid
int raspivid_pid = 0;

//sound
int sound_interr = 0;
int sound_flag = 0;

//pir
int pir_interr = 0;
int pir_flag = 0;

//temp
int low_temp_flag = 0;
float low_temp_limit = -1;

int upp_temp_flag = 0;
float upp_temp_limit = -1;

string get_ip(void)
{
	system("hostname -I > ip.txt");
	FILE* ip = fopen("ip.txt", "r");
	char result[24]={0x0};

	fgets(result, sizeof(result), ip);
	int size = sizeof(result) / sizeof(char);

	string r_ip = "";
	for (int i = 0; i < size; i++)
	{
		r_ip+= result[i];
	}
	pclose(ip);

	return r_ip;
}

int get_pid(void)
{
	FILE *cmd = popen("pgrep raspivid", "r");
	char result[24]={0x0};

	fgets(result, sizeof(result), cmd);
	raspivid_pid = atoi(result);
	cout << "raspivid_pid = " << raspivid_pid << endl;
	pclose(cmd);

	return raspivid_pid;
}

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
	cout << "move detected!" << endl;
	if(pir_flag == 0)
	{
		system("echo \"Move detected!\" | mail -s \"#RPi Move alert!\" MyPi1289@gmail.com");
	}
	pir_flag = 1;

	pir_interr = 1;
}

void sound_interrupt(void)
{
	cout << "noise detected!"<< endl;
	if(sound_flag == 0)
	{
		system("echo \"Noise detected!\" | mail -s \"#RPi Noise alert!\" MyPi1289@gmail.com");
	}
	sound_flag = 1;

	sound_interr = 1;
}

int temp_hot(int &handle, int upp_temp_limit)
{
	float current_temp = getTemperature(handle,TEMP_REG);

	if(current_temp >= upp_temp_limit)
	{
		cout << "current_temp = " << FIXED_FLOAT(current_temp) << " upp_temp_limit = " << FIXED_FLOAT(upp_temp_limit) << endl;
		cout<<"Temperature too high!"<<endl;

		if(upp_temp_flag == 0)
		{
			system("echo \"Temperature too high!\" | mail -s \"#RPi High temperature alert!\" MyPi1289@gmail.com");
		}
		upp_temp_flag = 1;

		return 1;
	}else
		return 0;
}

int temp_cold(int &handle, int low_temp_limit)
{
	float current_temp = getTemperature(handle,TEMP_REG);

	if(current_temp <= low_temp_limit)
	{
		cout << "current_temp = " << FIXED_FLOAT(current_temp) << " low_temp_limit = " << FIXED_FLOAT(low_temp_limit) << endl;
		cout<<"Too cold"<<endl;

		if(low_temp_flag == 0)
		{
			system("echo \"Temperature too low!\" | mail -s \"#RPi Low temperature alert!\" MyPi1289@gmail.com");
		}
		low_temp_flag = 1;

		return 1;
	}else
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
    	wiringPiI2CWriteReg16(handle, T_UP_REG,0x9101); //upper temp treshold set to 23 C
    	wiringPiI2CWriteReg16(handle, T_DOWN_REG,0x2101); //lower temp treshold set to 21 C

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
        raspivid_pid = get_pid();

    	if(raspivid_pid > 0)
    	{
        	delay(1000);
    		system("pkill -USR1 raspivid"); //send signal to stop capturing - end of initialization
        	//system("pkill -9 raspivid"); //killing raspivid process
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
	//IP
	string ip = get_ip();
	cout<< "current IP " << ip << endl;

	int counter = 0;
	int camera_status = 0;
	int handle = wiringPiI2CSetup(DEV_ADDR);
	int fd = wiringPiSetup();

    if (setup(handle, fd))
    {
    	cout << "Waiting for any event" << endl;
    	while(1)
    	{
    		if (sound_interr || pir_interr || temp_hot(handle, upp_temp_limit) == 1 || temp_cold(handle, low_temp_limit) == 1)
    		{
    			sound_interr = 0;
    			pir_interr = 0;
    			counter = 0;	//each event clears counter in order to continue recording

    			if (camera_status == 0 && raspivid_pid > 0) //enters here only during first loop
    			{
    				cout << "Start capturing the video" << endl;
    				system("pkill -USR1 raspivid");
    				camera_status = 1;

    			}else if(camera_status == 0 && raspivid_pid == 0) //enters here after killig process
    			{
    				cout << "Start capturing the video" << endl;
    				system("bash -c \"(raspivid -s -vf -o - -t 0 -n -w 320 -h 240 -fps 24 &) | (tee -a /home/pi/win_share/test_video.h264 &) | (cvlc -vvv stream:///dev/stdin --sout '#rtp{sdp=rtsp://:8000/}' :demux=h264 &)\"");
    				raspivid_pid = get_pid();
    				camera_status = 1;
    			}
    			cout << "Waiting for next event" << endl;
    		}
    		delay(500); //delay for decreasing cpu load (0,5 S)

    		if (camera_status)
    			++counter; //camera should run for period of time after last event

    		if (counter > 120) //240 x 0,5 s = 2 min
    		{

    			cout << "Stop capturing the video" << endl;
    			system("pkill -9 raspivid");

    			raspivid_pid = 0;
    			camera_status = 0;

    			//timing
    			counter = 0;

    			// sensors flags
    			sound_flag = 0;
    			pir_flag = 0;
    			low_temp_flag = 0;
    			upp_temp_flag = 0;

    			//uploading file on nextcloud
    			string cmd("curl -k -u \"ola1289:Fiolek12345\" -T /home/pi/win_share/test_video.h264 -H 'X-Method-Override: PUT' https://");
    			cmd+=ip;
    			cmd+="/nextcloud/remote.php/dav/files/ola1289/";
    			system(cmd.c_str());

    			//removing file from disc - file is not overwriting so it would get too big in time
    			system("rm /home/pi/win_share/test_video.h264");

    		}
    	}
    }
    else
    {
    	cout << "SETUP FAILURE" << endl;
    }

    return 0;
}
