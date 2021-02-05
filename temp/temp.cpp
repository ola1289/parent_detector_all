#include <stdio.h>
#include <stdlib.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <time.h>
#include <bitset>
#include <iostream>

#define DEV_ADDR 0x18
#define TEMP_REG 0x05
#define T_CONF_REG 0x01
#define T_UP_REG 0x02
#define T_DOWN_REG 0x03
#define T_CRIT_REG 0x04

using namespace std;

int getTemperature(int dev_addr, int reg_addr)
{
	int temp_reg = wiringPiI2CReadReg16(dev_addr, reg_addr);
	int t = temp_reg;
	int M_sb = temp_reg & 0x000f; //clear <15-13> bits
	int L_sb = (t & 0xff00)>>8;

	printf("%d, %d\n", M_sb*16, L_sb/16);

	int temp = (M_sb*16) + (L_sb/16);
	printf("%d\n", temp);

	return temp;
}

int main (void)
{
	//dopisac if setup
	int handle = wiringPiI2CSetup(DEV_ADDR);

	//write
	wiringPiI2CWriteReg16(handle,T_UP_REG,0x9101);//ustawienie górnego pu³apu temp 25 C
	wiringPiI2CWriteReg16(handle,T_DOWN_REG,0x2101); //ustawienie dolnego pu³apu temp 18 C

	int upp_temp_limit = getTemperature(handle,T_UP_REG);
	int low_temp_limit = getTemperature(handle,T_DOWN_REG);

	while(true)
	{
		int current_temp = getTemperature(handle,TEMP_REG);


		if(current_temp >= upp_temp_limit)
		{
			cout<<"Too hot"<<endl;
		}else if(current_temp <= low_temp_limit)
		{
			cout<<"Too cold"<<endl;
		}else
		{
			cout<<"Temp ok"<<endl;;
		}
		delay(3000);
	}


	return 0 ;
}
