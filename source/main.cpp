/**************************************************************
    KOALA CRAWLER  CANOPEN  PAYLOAD MODULE
**************************************************************/
/*

Copyright (C): Alessandro De Crescenzo - ale-dc@live.it

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "mbed.h"
#include "main.h"
#include "canfestival.h"
#include "can_mbed.h"
#include "koala_OD.h"
#include "koalaPeriph.h"
//#include "port_helper.h"



DigitalOut gled(LED1); //onboard led for debug
// Set used for main program timing control
Ticker SysTimer;
//Ticker MeasurementTimer;
// flag used by the main program timing control interrupt
volatile uint8_t timer_interrupt = 0;
volatile uint8_t meas_irq = 0;
volatile uint8_t change_node_id = 0;

// CAN - put and take data from the stack
uint8_t nodeID;
//uint8_t digital_input[1] = {0};
//uint8_t digital_output[1] = {0};
//uint8_t last_digital_output;

// read a can message from the stack
static Message m = Message_Initializer;

int main() 
{
   
    // initialize the helper code - just for debugging
    //initHelper();
    init_uart();
    printf("Start can...\n");//TODO remove
    //printf("cpu freq: %u \n",SystemCoreClock);
    // start the system timer 
    SysTimer.attach_us(&serviceSysTimer, CYCLE_TIME);

    //MeasurementTimer.attach(&measurement_isr, 0.250); //250 ms for thermal readings



    // start of CANfestival stack calls
    canInit(CAN_BAUDRATE);              // Initialize the CANopen bus
    initTimer();                        // Start timer for the CANopen stack
    nodeID = 1;                           // node id can be anything (1-127)
    setNodeId (&mbed_slave_Data, nodeID);
    // Init the state           
    setState(&mbed_slave_Data, Initialisation);
    
    init_stepper();
    init_led();

    wait(0.1);
    
    // just keep loopin'
    while(1){
        // if(meas_irq){
        //     meas_irq =0;    
        //     thermal_measurement_fun();
        // }
        // Cycle timer, invoke action on every time slice
        if (sys_timer){ 
            // Reset timer
            reset_sys_timer(); 

            /* service Koala peripherials inputs and outputs */
            stepper_handler(&mbed_slave_Data);
            led_handler(&mbed_slave_Data);
            
            gled = !gled;            
        }

        // a message was received - pass it to the CANstack
        if (canReceive(&m)){     
            // interrupts need to be disabled here
            __disable_irq();
            // process it
            canDispatch(&mbed_slave_Data, &m);
            // and re-enabled here
            __enable_irq();
            // print it to the console for debugging
            //printMsg(m); //just debug remove in release
        }
    }
}

// ISR for the Ticker
void serviceSysTimer()
{
    timer_interrupt = 1;
}


// ISR for torque meas
// void measurement_isr()
// {
//     meas_irq = 1;
// }

