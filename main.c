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
int ADC_reading(int channel) {
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


    *(ch1) = 0x1;//Sets channel into auto-update
    (*direction_gpio) = 0x3FF;// sets gpio as outputs
	(*dat_gpio) = 0x0; //clear outputs


    while(1) {

        int statuspushbutton = *pushb ;
        int prev_status;

        //set curtains to manual update
        if(switch_read() == 0) {
            //if enough time possibly make 3 button system that allows user to start, stop, and close curtains would not use loops in that scenario    
            
            //if open button is pressed
            if(statuspushbutton == 1 && prev_status ==0){
                //code to make curtains come down
                //possibly put code on loop to prevent anything else done by the user to affect it until curtain is fully open
            }else if (statuspushbutton==2 && prev_status ==0){
                //code to make curtains come up
                //possibly put code on loop to prevent anything else done by the user to affect it until curtain is fully close
            }

        }
        //set curtains to automatic
        else {

            //read ADC values to simulate photoresistors analog readings
            ADC_read = ADC_reading();

            //treshold to simulate when its dark outside
            //values go between (0-4096) if dark low values if high then its light outside
            if (ADC_read< 1500){
                //code simulating motor closing curtain


            }else{// its sunny outside
                //code simulating motor opening curtain

            }

        }
        prev_status = statuspushbutton;
    }
}
