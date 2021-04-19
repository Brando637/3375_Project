#include "address_map_arm.h"
#include <stdio.h>

//Set pointers to board peripherals
volatile unsigned int *dat_gpio = (unsigned int*) JP1_BASE;
volatile unsigned int *direction_gpio = (unsigned int*) (JP1_BASE+0x04);
volatile unsigned int * ch0 = (unsigned int*) ADC_BASE;
volatile int * switch_ptr = (int*)SW_BASE;
volatile int * pushb = (int*)KEY_BASE;

//Read switch state
int switch_read(void){
   int switchst = *switch_ptr;
   switchst = switchst & 0x11;//Reading from the first two switches
   return(switchst);
}

//Activate Stepper Motor
void step(int rotate)
{
    //Rotate the motor clockwise
    if(rotate == 0)
    {
        *dat_gpio = 0x09;
        *dat_gpio = 0x0c;
        *dat_gpio = 0x06;
        *dat_gpio = 0x03;
    }

    //Rotate the motor counterclockwise
    else
    {
        *(dat_gpio) = 0x03;
        *(dat_gpio) = 0x06;
        *(dat_gpio) = 0x0c;
        *(dat_gpio) = 0x09;
    }
}

int main(void) {


    //initialize variables
    unsigned int ADC_data;
    volatile int delay_count;
    int prev_status=0;
    int executing = 0;
    int open = 0;
    int close = 0;

    int interval = 100000000;//1 second
    int totalTime = 0;//Holds the total time that has passed in seconds
    int bright, dark, critPnt;//Values to be used by when in automatic mode
    bright = dark = critPnt = 0;
    int rotations = 500;//Change this value to the number of rotations needed fully unfurl and furl the blinds

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


    
    (*direction_gpio) = 0x3FF;// sets gpio as outputs
	*dat_gpio = 0x0; //clear outputs


    while(1) {
        //get status of the pushbutton
        int statuspushbutton = *pushb ;

        *(ch0) = 0x0;//Sets all channels to update and starts ADC read


        //Set curtains to manual update
        if(switch_read() == 0x0) {
            //if enough time possibly make 3 button system that allows user to start, stop, and close curtains would not use loops in that scenario

            //if open button is pressed
            if((statuspushbutton&1) == 1 && prev_status==8 && executing == 0){
                //code to make curtains come down
                int i;
                for( i = 0; i > rotations; i++)
                {
                    step(0);//Rotate the motor clockwise
                }
                open =1;
                executing = 1;
            //if close button is pressed
            }else if ((statuspushbutton&2)==2 && prev_status ==4 && executing == 0){
                //code to make curtains come up
                int i;
                for( i = 0; i > rotations; i++)
                {
                    step(1);//Rotate the motor 
                }
                close = 1;
                executing = 1;
            }
        }

        //Set curtains to automatic
        else if (switch_read() == 0x1) {

            //Read ADC values to simulate photoresistors analog readings while the blinds are not in motion
            if (executing==0){
                ADC_data = *(ch0);
            }

            //If the current ADC reading measure gets to the critical point then the blinds need to close 
            if (ADC_data <= critPnt){
                //code simulating motor closing blinds
                if(((statuspushbutton & 8) != 8) && executing == 0){
                    int i;
                    for ( i = 0; i > rotations; i++)
                    {
                        step(1); //Rotate the motor clockwise
                    }
                    executing=1;
                    close= 1;
                }

            }else{// its sunny outside so the blinds should be open
                //code simulating motor opening blinds
                if(((statuspushbutton&4) != 4) && executing == 0 ){
                    int i;
                    for ( i = 0; i > rotations; i++)
                    {
                        step(0); //Rotate the motor clockwise
                    }
                    executing=1;
                    open= 1;
                }
            }
        }

        //Set curtains to timer mode
        else if (switch_read() == 0x2)
        {
            int openState = 0;
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
                            int i;
                            for ( i = 0; i > rotations; i++)
                            {
                                step(openState); //Rotate the motor clockwise or coutnerclockwise
                            }
                            
                            //Change the state for opening or closing the blinds
                            if(openState == 0)
                            { openState = 1; }
                            if(openState == 1)
                            {openState =0;}
                        }
                    }
                }

        }

        //Setup mode for setting brightess point in day and darkest point in day
        else if(switch_read() == 0x3)
        {
            //User wants to set the current brightness as the brightest point in the day
            if(statuspushbutton == 0x1)
            {
                bright = *(ch0);
            }

            //User wants to set the current brightness as the dimmest point in the day
            if(statuspushbutton = 0x2)
            {
                dark = *(ch0);
            }

            //Set the critical point which is used to cause the blinds to open or close
            if( (bright != 0 && dark != 0) && (bright > dark) )
            {
                //The critical point is when the value gets to a 30% 
                critPnt = (bright - dark) * 0.3;
            }
        }
        //set prevstate to current state
        prev_status = statuspushbutton;
    }
}
