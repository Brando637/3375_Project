#include "address_map_arm.h"
#include <stdio.h>

//set pointers to board peripherals
volatile unsigned int *dat_gpio = (unsigned int*) JP1_BASE;
volatile unsigned int *direction_gpio = (unsigned int*) (JP1_BASE+0x04);
volatile unsigned int * ch1 = (unsigned int*) (ADC_BASE+0x04);
volatile int * switch_ptr = (int*)SW_BASE;
volatile int * pushb = (int*)KEY_BASE;

//read switch state
int switch_read(void){
   int switchst = *switch_ptr;
   switchst = switchst & 1;
   return(switchst);
}
//read ADC to simuate photoresistor
int ADC_reading(void) {
    volatile unsigned int *channel_reading;
    int reading;

    channel_reading = (unsigned int*) ADC_BASE;

    //rturm 12-bit data from channel
    reading = *channel_reading & 0xFFF;
	return reading;
}
int main(void) {


    //initialize variables
    volatile int ADC_read;
    volatile int delay_count;
    int prev_status=0;
    int executing = 0;
    int open = 0;
    int close = 0;


    *(ch1) = 0x1;//Sets channel into auto-update
    (*direction_gpio) = 0x3FF;// sets gpio as outputs
	*dat_gpio = 0x0; //clear outputs


    while(1) {
        //get status of the pushbutton
        int statuspushbutton = *pushb ;


        //set curtains to manual update
        if(switch_read() == 0) {
            //if enough time possibly make 3 button system that allows user to start, stop, and close curtains would not use loops in that scenario

            //if open button is pressed
            if((statuspushbutton&1) == 1 && prev_status==8 && executing == 0){
                //code to make curtains come down
                *dat_gpio = 0x01;//run motor clockwise
                open =1;
                executing = 1;
            //if close button is pressed
            }else if ((statuspushbutton&2)==2 && prev_status ==4 && executing == 0){
                //code to make curtains come up
                *dat_gpio = 0x02;//run motor counterclockwise
                close = 1;
                executing = 1;
            //if button at bottom of window is pressed by the curtain
            }else if (((statuspushbutton & 4) == 4) && open == 1 && executing ==1){
                *dat_gpio = 0x0;//stop motor
                executing= 0;
                open =0;
            //if button at the top of window is pressed by the curtain
            }else if(((statuspushbutton & 8) == 8) && close == 1 && executing ==1){
                *dat_gpio = 0x0;
                executing = 0;
                close = 0;
            }

        }
        //set curtains to automatic
        else {

            //read ADC values to simulate photoresistors analog readings while curtain is not in motion
            if (executing==0){
                ADC_read = ADC_reading();
            }


            //treshold to simulate when its dark outside
            //values go between (0-4096) if dark low values if high then its light outside
            if (ADC_read< 1500){
                //code simulating motor closing curtain
                if(((statuspushbutton & 8) != 8) && executing == 0){
                    *dat_gpio = 0x02;//run motor counterlockwise
                    executing=1;
                    close= 1;
                }
                //if button at the bottom of window is pressed by the curtain
                else if(((statuspushbutton & 8) == 8) && close == 1 && executing ==1){
                    *dat_gpio = 0x0;//stop motor
                    executing = 0;
                    close = 0;
                }

            }else{// its sunny outside so curtains should be open
                //code simulating motor opening curtain
                if(((statuspushbutton&4) != 4) && executing == 0 ){
                    *dat_gpio = 0x01;//run motor clockwise
                    executing=1;
                    open= 1;
                //if button at the top of window is pressed by the curtain then stop motor
                }else if(((statuspushbutton & 4) == 4) && open == 1 && executing ==1){
                    *dat_gpio = 0x0;//stop motor
                    executing = 0;
                    open = 0;
                }
            }
        for (delay_count = 300000; delay_count != 0; --delay_count);
        }
        //set prevstate to current state
        prev_status = statuspushbutton;
    }
}
