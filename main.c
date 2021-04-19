#include "address_map_arm.h"
#include <stdio.h>

//Set pointers to board peripherals
volatile unsigned int *dat_gpio = (unsigned int*) JP1_BASE;
volatile unsigned int *direction_gpio = (unsigned int*) (JP1_BASE+0x04);
volatile unsigned int * ch1 = (unsigned int*) (ADC_BASE+0x04);
volatile int * switch_ptr = (int*)SW_BASE;
volatile int * pushb = (int*)KEY_BASE;

//Read switch state
int switch_read(void){
   int switchst = *switch_ptr;
   switchst = switchst & 0x11;//Reading from the first two switches
   return(switchst);
}
//Read ADC to simluate photoresistor
int ADC_reading(void) {
    volatile unsigned int *channel_reading;
    int reading;

    channel_reading = (unsigned int*) ADC_BASE;

    //return 12-bit data from channel
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

    int interval = 100000000;//1 second
    int totalTime = 0;//Holds the total time that has passed in seconds

    typedef struct _interval_timer
    {
        int status;
        int control;
        int low_period;
        int high_period;
        int low_counter;
        int high_counter;
    } interval_timer;

    volatile interval_timer* const timer_1_ptr = (interval_timer*)TIMER_BASE;


    *(ch1) = 0x1;//Sets channel into auto-update
    (*direction_gpio) = 0x3FF;// sets gpio as outputs
	*dat_gpio = 0x0; //clear outputs


    while(1) {
        //get status of the pushbutton
        int statuspushbutton = *pushb ;


        //Set curtains to manual update
        if(switch_read() == 0x0) {
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

        //Set curtains to automatic
        else if (switch_read() == 0x1) {

            //Read ADC values to simulate photoresistors analog readings while curtain is not in motion
            if (executing==0){
                ADC_read = ADC_reading();
            }


            //Threshold to simulate when its dark outside
            //Values go between (0-4096) if dark low values if high then its light outside
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

        //Set curtains to timer mode
        else if (switch_read() == 0x2)
        {
            //Stop the timer incase it was running previously;
            timer_1_ptr -> control = 0x8;
            
            //Clear the timer just in case there was something there previously
            timer_1_ptr -> status = 0xa;

            //Clear the total time elapsed
            totalTime = 0;

            //Start the timer
                //We'll set the interval for the timer first
                timer_1_ptr -> low_period = interval;
                timer_1_ptr -> high_period = interval >> 16;

                //Actually start the timer
                timer_1_ptr -> control = 0x6;

                //Continue going on with the timer as long as the switch combination is 0x10
                while(switch_read() == 0x2)
                {
                    //Check if we have met the timeout for the interval
                    if(timer_1_ptr -> status == 0x3)
                    {
                        totalTime += 1;//1s has passed
                        //Clear the timer
                        timer_1_ptr -> status = 0xa;

                        //Check to see if the total time that has passed is 28800 seconds(8 hours) 
                        if(totalTime == 28800)
                        {
                            //Reset the total time to 0
                            totalTime = 0;
                        }

                    }
                }

        }
        //set prevstate to current state
        prev_status = statuspushbutton;
    }
}
